#include "dialogselect.h"
#include "ui_dialogselect.h"

DialogSelect::DialogSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelect)
{
    ui->setupUi(this);

    m_result = 0;
}

DialogSelect::~DialogSelect()
{
    delete ui;
}

void DialogSelect::accept()
{
    if(ui->radioFromLocal->isChecked())
        m_result = 0;
    else
        m_result = 1;

    QDialog::accept();
}

int DialogSelect::get_result()
{
    return m_result;
}
