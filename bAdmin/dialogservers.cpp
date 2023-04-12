#include "dialogservers.h"
#include "ui_dialogservers.h"
#include <QMessageBox>
#include <QInputDialog>

DialogServers::DialogServers(QStringList& list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogServers)
{
    ui->setupUi(this);
    auto  m_list = new QStringListModel(list);
    ui->tableView->setModel(m_list);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWindowTitle("Сервера");
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->hide();
    list_ = list;

}

DialogServers::~DialogServers()
{
    delete ui;
}

QStringList DialogServers::getResult() const
{
    return list_;
}

QString DialogServers::selected()
{
    return m_selected;
}

void DialogServers::accept()
{
    QDialog::accept();
}

void DialogServers::on_tableView_doubleClicked(const QModelIndex &index)
{

}

void DialogServers::on_btnSelect_clicked()
{

    auto currentIndex = ui->tableView->currentIndex();
    if(!currentIndex.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент");
        return;
    }
    m_selected = currentIndex.data().toString();

    accept();
}


void DialogServers::on_btnAdd_clicked()
{
    QString result = QInputDialog::getText(this, "Добавить сервер", "Введите url сервера");

    if(!result.isEmpty()){
        auto model = (QStringListModel)ui->tableView->model();
        list_.append(result);
        ui->tableView->setModel(nullptr);
        ui->tableView->setModel(new QStringListModel(list_));
        ui->tableView->resizeColumnsToContents();
    }
}


void DialogServers::on_btnDelete_clicked()
{
    auto index = ui->tableView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = ui->tableView->model();
    ui->tableView->model()->removeRow(index.row());

    list_.remove(list_.indexOf(model->index(index.row(), 0).data().toString()));

}



