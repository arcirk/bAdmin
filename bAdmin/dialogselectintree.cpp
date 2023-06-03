#include "dialogselectintree.h"
#include "ui_dialogselectintree.h"
#include <QMessageBox>

DialogSelectInTree::DialogSelectInTree(TreeViewModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectInTree)
{
    ui->setupUi(this);

    ui->treeView->setModel(model);

    for (int i = 0; i < model->columnCount(QModelIndex()); ++i) {
        auto key = model->get_column_name(i);
        if(key != "name" && key != "size"){
            ui->treeView->hideColumn(i);
        }
    }

    allow_sel_group_ = false;

    model->reset();
    ui->treeView->resizeColumnToContents(0);


}

DialogSelectInTree::DialogSelectInTree(TreeViewModel *model, QVector<QString> hideColumns, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectInTree)
{
    ui->setupUi(this);

    ui->treeView->setModel(model);

    foreach (auto v, hideColumns) {
        auto i = model->get_column_index(v);
        if(i != -1)
            ui->treeView->hideColumn(i);
    }

    allow_sel_group_ = false;

    model->reset();
    ui->treeView->resizeColumnToContents(0);
}

//DialogSelectInTree::DialogSelectInTree(arcirk::server::server_objects srvObject, QVector<QString> hideColumns, QWidget *parent)
//{

//}

DialogSelectInTree::~DialogSelectInTree()
{
    delete ui;
}

QString DialogSelectInTree::file_name() const
{
    return file_name_;
}

void DialogSelectInTree::accept()
{
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Элемент не выбран!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeView->model();
    auto in = model->get_column_index("path");
    if(in != -1)
        file_name_ = ui->treeView->model()->index(index.row(), in).data().toString();

    sel_object = model->get_object(index);

    if(!allow_sel_group_){
        auto is_find = sel_object.find("is_group") != sel_object.end();
        if(is_find){
            auto is_group = sel_object["is_group"].get<int>();
            if(is_group){
                QMessageBox::critical(this, "Ошибка", "Выберете элемент!");
                return;
            }
        }

    }

    QDialog::accept();
}

nlohmann::json DialogSelectInTree::selectedObject()
{
    return sel_object;
}

void DialogSelectInTree::allow_sel_group(bool value)
{
    allow_sel_group_ = value;
}

void DialogSelectInTree::on_treeView_doubleClicked(const QModelIndex &index)
{
    accept();
}


void DialogSelectInTree::on_buttonBox_accepted()
{
    accept();
}


void DialogSelectInTree::on_buttonBox_rejected()
{
    QDialog::close();
}

