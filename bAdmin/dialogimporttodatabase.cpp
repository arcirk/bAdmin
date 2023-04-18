#include "dialogimporttodatabase.h"
#include "ui_dialogimporttodatabase.h"

DialogImportToDatabase::DialogImportToDatabase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogImportToDatabase)
{
    ui->setupUi(this);
}

DialogImportToDatabase::~DialogImportToDatabase()
{
    delete ui;
}
