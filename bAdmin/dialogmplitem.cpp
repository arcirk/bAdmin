#include "dialogmplitem.h"
#include "ui_dialogmplitem.h"
#include <QStringListModel>

DialogMplItem::DialogMplItem(mpl_item& item, const QStringList& profiles, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMplItem),
    item_(item),
    profiles_(profiles)
{
    ui->setupUi(this);

    load_data();
}

DialogMplItem::~DialogMplItem()
{
    delete ui;
}

void DialogMplItem::accept()
{
    item_.profile = ui->cmbMozillaProfile->currentText().toStdString();
    item_.name = ui->txtProfileName->text().toStdString();
    item_.url = ui->txtMplItemUrl->toPlainText().toStdString();

    QDialog::accept();
}

void DialogMplItem::setProfilesList()
{
    if(!profiles_.isEmpty()){
        auto model = new QStringListModel(profiles_);
        ui->cmbMozillaProfile->setModel(model);
    }

    emit profilesChanged(profiles_);
}

void DialogMplItem::load_data()
{
    if(!profiles_.isEmpty()){
        auto model = new QStringListModel(profiles_);
        ui->cmbMozillaProfile->setModel(model);
    }

    ui->cmbMozillaProfile->setCurrentText(item_.profile.c_str());
    ui->txtProfileName->setText(item_.name.c_str());
    ui->txtMplItemUrl->setText(item_.url.c_str());
}

void DialogMplItem::mozillaProfiles(const QStringList &profiles)
{
    profiles_ = profiles;
    setProfilesList();
}


void DialogMplItem::on_btnResetProfiles_clicked()
{
    emit getMozillaProfiles();
}

