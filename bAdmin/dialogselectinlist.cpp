#include "dialogselectinlist.h"
#include "ui_dialogselectinlist.h"
#include <QStringListModel>
#include <QMessageBox>

DialogSelectInList::DialogSelectInList(const QStringList& list, const QString& title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectInList)
{
    ui->setupUi(this);
    auto  m_list = new QStringListModel(list);
    ui->tableView->setModel(m_list);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWindowTitle(title);
    ui->tableView->resizeColumnsToContents();
}

DialogSelectInList::DialogSelectInList(QAbstractTableModel *list, const QString &title, QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogSelectInList)

{
    ui->setupUi(this);

    ui->tableView->setModel(list);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWindowTitle(title);
    ui->tableView->resizeColumnsToContents();
}

DialogSelectInList::DialogSelectInList(QAbstractItemModel *list, const QString &title, QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogSelectInList)
{
    ui->setupUi(this);

    ui->tableView->setModel(list);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWindowTitle(title);
    ui->tableView->resizeColumnsToContents();
}

DialogSelectInList::DialogSelectInList(QAbstractItemModel *list, const QString &title, QList<int> visibleColumns, QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogSelectInList)
{
    ui->setupUi(this);

    ui->tableView->setModel(list);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWindowTitle(title);
    if(visibleColumns.size() > 0){
        for (int i = 0; i < list->columnCount(); ++i) {
            if(visibleColumns.indexOf(i) == -1){
                ui->tableView->setColumnHidden(i, true);
            }
        }
    }
    ui->tableView->resizeColumnsToContents();
}

DialogSelectInList::DialogSelectInList(QAbstractTableModel *list, const QString &title, QList<int> visibleColumns, QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogSelectInList)
{
    ui->setupUi(this);

    ui->tableView->setModel(list);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWindowTitle(title);

    if(visibleColumns.size() > 0){
        for (int i = 0; i < list->columnCount(); ++i) {
            if(visibleColumns.indexOf(i) == -1){
                ui->tableView->setColumnHidden(i, true);
            }
        }
    }

    ui->tableView->resizeColumnsToContents();
}

DialogSelectInList::~DialogSelectInList()
{
    delete ui;
}

void DialogSelectInList::accept()
{

    auto index = ui->tableView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = ui->tableView->model();

    for (int i = 0; i < model->columnCount(); ++i) {
        _result.append(model->index(index.row(), i).data().toString());
    }

    QDialog::accept();
}

QStringList DialogSelectInList::dialogResult()
{
    return _result;
}

void DialogSelectInList::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(index.isValid())
        accept();
}

