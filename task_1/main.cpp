#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include "treeapp.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    QWidget *centralWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    TreeApp *treeApp = new TreeApp;
    layout->addWidget(treeApp);

    // Создаем кнопки внизу
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton("Добавить");
    QPushButton *editButton = new QPushButton("Редактировать");
    QPushButton *deleteButton = new QPushButton("Удалить");
    QPushButton *loadButton = new QPushButton("Загрузить из JSON");
    QPushButton *saveButton = new QPushButton("Сохранить в JSON");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(saveButton);
    layout->addLayout(buttonLayout);

    mainWindow.setCentralWidget(centralWidget);
    mainWindow.resize(400, 600);
    mainWindow.show();

    // Подключаем кнопки к методам TreeApp
    QObject::connect(addButton, &QPushButton::clicked, treeApp, &TreeApp::addItem);
    QObject::connect(editButton, &QPushButton::clicked, treeApp, &TreeApp::editItem);
    QObject::connect(deleteButton, &QPushButton::clicked, treeApp, &TreeApp::deleteItem);
    QObject::connect(loadButton, &QPushButton::clicked, treeApp, &TreeApp::loadTreeFromJson); // Загрузка дерева
    QObject::connect(saveButton, &QPushButton::clicked, treeApp, &TreeApp::saveTreeToJson); // Сохранение дерева

    return app.exec();
}
