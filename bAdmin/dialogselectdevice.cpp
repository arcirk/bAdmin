#include "dialogselectdevice.h"
#include "ui_dialogselectdevice.h"


DialogSelectDevice::DialogSelectDevice(QWidget *parent, const QStringList& labels, const QString& tTitle) :
    QDialog(parent),
    ui(new Ui::DialogSelectDevice)
{
    ui->setupUi(this);

    ctrl.append(ui->radio1);
    ctrl.append(ui->radio2);
    ctrl.append(ui->radio3);
    ctrl.append(ui->radio4);

    for (int i = 0; i < ctrl.size(); ++i) {
        if(labels.size() > i){
            ctrl[i]->setText(labels[i]);
        }else{
            ctrl[i]->setVisible(false);
        }
    }

    ctrl[0]->setChecked(true);

    _currentSelection = -1;

    this->adjustSize();

    setWindowTitle(tTitle);
}

DialogSelectDevice::~DialogSelectDevice()
{
    delete ui;
}

void DialogSelectDevice::accept()
{

    QRadioButton* r;

    foreach(auto ratio, ctrl){
        if(ratio->isChecked())
            r = ratio;
    }
    if(r)
        _currentSelection = ctrl.indexOf(r);

    QDialog::accept();
}

int DialogSelectDevice::currentSelection()
{
    return _currentSelection;
}


