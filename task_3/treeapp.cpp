#include "treeapp.h"
#include "nodelevel.h"
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFont>
#include <QMessageBox>
#include <QMimeData>
#include <QDropEvent>
#include <QDir>
#include <QFileDialog>

TreeApp::TreeApp(QWidget *parent) : QTreeWidget(parent) {
    setHeaderLabel("Факультет, Группы и Студенты");
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragDropMode(QAbstractItemView::InternalMove);

    // Enable checkboxes
    setUniformRowHeights(true); // Optional for better appearance
    setAlternatingRowColors(true); // Optional for better appearance

    // Load tree from JSON on startup
    loadTreeFromJson();
}

void TreeApp::applyItemStyles(QTreeWidgetItem *item, QTreeWidgetItem *parent) {
    if (!parent) {
        // Faculty
        item->setFont(0, QFont("Arial", 14, QFont::Bold));
        item->setIcon(0, QIcon(":/recources/icons/resources/icons/faculty-icon.png"));
    } else if (!parent->parent()) {
        // Group
        item->setFont(0, QFont("Arial", 12));
        item->setIcon(0, QIcon(":/recources/icons/resources/icons/students-group.png"));
    } else {
        // Student
        item->setFont(0, QFont("Arial", 10));
        item->setIcon(0, QIcon());
    }
    // Make the item checkable
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Unchecked); // Initialize as unchecked
}

void TreeApp::saveCheckedItemsToFile() {
    // Open a dialog to choose the save location
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить выбранные элементы", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    for (int i = 0; i < topLevelItemCount(); ++i) {
        QTreeWidgetItem *facultyItem = topLevelItem(i);
        // Check if faculty item is checked
        if (facultyItem->checkState(0) == Qt::Checked) {
            out << facultyItem->text(0) << "\n";
        }
        for (int j = 0; j < facultyItem->childCount(); ++j) {
            QTreeWidgetItem *groupItem = facultyItem->child(j);
            if (groupItem->checkState(0) == Qt::Checked) {
                out << "  " << groupItem->text(0) << "\n";
            }
            for (int k = 0; k < groupItem->childCount(); ++k) {
                QTreeWidgetItem *studentItem = groupItem->child(k);
                if (studentItem->checkState(0) == Qt::Checked) {
                    out << "    " << studentItem->text(0) << "\n";
                }
            }
        }
    }

    file.close();
}


// Метод для обработки событий перетаскивания
void TreeApp::dropEvent(QDropEvent *event) {
    QTreeWidgetItem *sourceItem = currentItem();
    QTreeWidgetItem *targetItem = itemAt(event->pos());
    QTreeWidgetItem *targetParent = targetItem->parent();

    // запрещено перетаскивать факультеты
    if (!targetParent) {
        event->ignore();
        return;
    }

    QTreeWidgetItem *sourceParent = sourceItem->parent();
    NodeLevel sourceNodeLevel, targetNodeLevel;

    if (sourceParent && sourceParent->parent() == nullptr) {
        sourceNodeLevel = NodeLevel::Group;
    } else {
        sourceNodeLevel = NodeLevel::Student;
    }

    if (targetParent && targetParent->parent() == nullptr) {
        targetNodeLevel = NodeLevel::Group;
    } else {
        targetNodeLevel = NodeLevel::Student;
    }

    if (sourceNodeLevel == targetNodeLevel && (sourceParent == targetParent || sourceParent == targetItem)) {
        QTreeWidget::dropEvent(event);
    } else {
        event->ignore(); // Запрещаем любое другое перемещение
    }
}

void TreeApp::addItem() {
    QTreeWidgetItem *selectedItem = currentItem();
    if (!selectedItem) return;

    bool ok;
    QString text = QInputDialog::getText(this, "Добавить элемент", "Введите имя:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        auto *newItem = new QTreeWidgetItem(selectedItem, QStringList(text));
        applyItemStyles(newItem, selectedItem);
        selectedItem->setExpanded(true);
    }
}

void TreeApp::editItem() {
    QTreeWidgetItem *selectedItem = currentItem();
    if (!selectedItem) return;

    bool ok;
    QString text = QInputDialog::getText(this, "Редактировать элемент", "Введите новое имя:", QLineEdit::Normal, selectedItem->text(0), &ok);
    if (ok && !text.isEmpty()) {
        selectedItem->setText(0, text);
    }
}

void TreeApp::deleteItem() {
    QTreeWidgetItem *selectedItem = currentItem();
    if (selectedItem && selectedItem->parent()) {
        delete selectedItem;
    }
}

void TreeApp::sortStudents() {
    for (int i = 0; i < topLevelItemCount(); ++i) {
        QTreeWidgetItem *facultyItem = topLevelItem(i);
        for (int j = 0; j < facultyItem->childCount(); ++j) {
            QTreeWidgetItem *groupItem = facultyItem->child(j);

            // Сортируем студентов по алфавиту внутри группы
            QList<QTreeWidgetItem*> studentItems;
            for (int k = 0; k < groupItem->childCount(); ++k) {
                studentItems.append(groupItem->takeChild(k));
            }

            std::sort(studentItems.begin(), studentItems.end(), [](QTreeWidgetItem *a, QTreeWidgetItem *b) {
                return a->text(0) < b->text(0);
            });

            // Добавляем отсортированные элементы обратно в группу
            for (QTreeWidgetItem *studentItem : studentItems) {
                groupItem->addChild(studentItem);
            }
        }
    }
}

void TreeApp::loadTreeFromJson() {
    // Очищаем существующее дерево
    clear();

    // Открываем диалог для выбора файла JSON
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл для загрузки", "", "JSON Files (*.json);;All Files (*)");
    if (fileName.isEmpty()) return; // Если файл не выбран, выходим

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для чтения: " + file.errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject rootObj = doc.object();
    QJsonArray faculties = rootObj["faculties"].toArray();
    for (const QJsonValue &facValue : faculties) {
        QJsonObject facObj = facValue.toObject();
        auto *facultyItem = new QTreeWidgetItem(this, QStringList(facObj["name"].toString()));
        applyItemStyles(facultyItem, nullptr);
        QJsonArray groups = facObj["groups"].toArray();
        for (const QJsonValue &grpValue : groups) {
            QJsonObject grpObj = grpValue.toObject();
            auto *groupItem = new QTreeWidgetItem(facultyItem, QStringList(grpObj["name"].toString()));
            applyItemStyles(groupItem, facultyItem);

            QJsonArray students = grpObj["students"].toArray();
            for (const QJsonValue &stuValue : students) {
                QTreeWidgetItem *studentItem = new QTreeWidgetItem(groupItem, QStringList(stuValue.toString()));
                applyItemStyles(studentItem, groupItem);
            }
        }
    }
}

void TreeApp::saveTreeToJson() {
    // Открываем диалог для выбора пути для сохранения файла
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить файл как", "", "JSON Files (*.json);;All Files (*)");
    if (fileName.isEmpty()) return; // Если файл не выбран, выходим

    QJsonArray faculties;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        QTreeWidgetItem *facultyItem = topLevelItem(i);
        QJsonObject facObj;
        facObj["name"] = facultyItem->text(0);

        QJsonArray groups;
        for (int j = 0; j < facultyItem->childCount(); ++j) {
            QTreeWidgetItem *groupItem = facultyItem->child(j);
            QJsonObject grpObj;
            grpObj["name"] = groupItem->text(0);

            QJsonArray students;
            for (int k = 0; k < groupItem->childCount(); ++k) {
                students.append(groupItem->child(k)->text(0));
            }
            grpObj["students"] = students;
            groups.append(grpObj);
        }
        facObj["groups"] = groups;
        faculties.append(facObj);
    }

    QJsonObject rootObj;
    rootObj["faculties"] = faculties;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString());
        return;
    }

    if (file.write(QJsonDocument(rootObj).toJson()) == -1) {
        QMessageBox::warning(this, "Ошибка", "Не удалось записать данные в файл.");
    }

    file.close();
}

void TreeApp::closeEvent(QCloseEvent *event) {
    saveTreeToJson();
    QTreeWidget::closeEvent(event);
}
