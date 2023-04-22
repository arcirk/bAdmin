#ifndef DIALOGSELECTINLIST_H
#define DIALOGSELECTINLIST_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QAbstractTableModel>
#include <QAbstractItemModel>

namespace Ui {
class DialogSelectInList;
}

class DialogSelectInList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectInList(const QStringList& list, const QString& title, QWidget *parent = nullptr);
    explicit DialogSelectInList(QAbstractTableModel* list, const QString& title, QWidget *parent = nullptr);
    explicit DialogSelectInList(QAbstractItemModel* list, const QString& title, QWidget *parent = nullptr);
    explicit DialogSelectInList(QAbstractItemModel* list, const QString& title, QList<int> visibleColumns, QWidget *parent = nullptr);
    explicit DialogSelectInList(QAbstractTableModel* list, const QString& title, QList<int> visibleColumns, QWidget *parent = nullptr);
    ~DialogSelectInList();

    void accept() override;

    QStringList dialogResult();

private slots:
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::DialogSelectInList *ui;
    QStringList _result;
};

#endif // DIALOGSELECTINLIST_H
