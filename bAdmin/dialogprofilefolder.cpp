#include "dialogprofilefolder.h"
#include "ui_dialogprofilefolder.h"
#include <QMessageBox>

DialogProfileFolder::DialogProfileFolder(TreeViewModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProfileFolder)
{
    ui->setupUi(this);

    ui->treeView->setModel(model);

    for (int i = 0; i < model->columnCount(QModelIndex()); ++i) {
        auto key = model->get_column_name(i);
        if(key != "name" && key != "size"){
            ui->treeView->hideColumn(i);
        }
    }

    model->reset();
    ui->treeView->resizeColumnToContents(0);

}

DialogProfileFolder::~DialogProfileFolder()
{
    delete ui;
}

QString DialogProfileFolder::file_name() const
{
    return file_name_;
}

void DialogProfileFolder::accept()
{
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран файл!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeView->model();
    auto in = model->get_column_index("path");
    file_name_ = ui->treeView->model()->index(index.row(), in).data().toString();

    QDialog::accept();
}
