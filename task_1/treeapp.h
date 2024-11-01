#ifndef TREEAPP_H
#define TREEAPP_H

#include <QTreeWidget>
#include <QJsonObject>

class TreeApp : public QTreeWidget {
    Q_OBJECT

public:
    TreeApp(QWidget *parent = nullptr);
    void loadTreeFromJson();
    void saveTreeToJson();

public slots:
    void addItem();
    void editItem();
    void deleteItem();

protected:
    void closeEvent(QCloseEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void applyItemStyles(QTreeWidgetItem *item, QTreeWidgetItem *parent);
    QJsonObject itemToJson(QTreeWidgetItem *item) const;
    void jsonToItem(const QJsonObject &obj, QTreeWidgetItem *parent);
};

#endif // TREEAPP_H
