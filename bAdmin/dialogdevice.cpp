#include "dialogdevice.h"
#include "ui_dialogdevice.h"
#include <QStringListModel>
#include <QComboBox>
#include <QUuid>

DialogDevice::DialogDevice(const nlohmann::json& deviceDetails, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDevice)
{
    ui->setupUi(this);

    result_ = pre::json::from_json<devices>(deviceDetails["device"]);
    auto tables_j = deviceDetails["tables"];
    if(tables_j.is_object()){
        auto items = tables_j.items();
        for (auto itr = items.begin(); itr != items.end(); ++itr) {
            json t_name = itr.key();
            auto tbl = t_name.get<tables>();
            auto rows = itr.value()["rows"];
            QMap<std::string, std::string> d;
            for (auto row = rows.begin(); row != rows.end(); ++row) {
                auto row_ = *row;
                d.insert(row_["first"].get<std::string>(), row_["ref"].get<std::string>());
            }
            d.insert("", arcirk::uuids::nil_string_uuid());
            tables_v.insert(tbl,d);
        }

    }

    for (auto itr = tables_v.begin(); itr != tables_v.end(); ++itr) {
        auto v = itr.value();
        QStringList lst;
        for (auto it = v.begin(); it != v.end(); ++it) {
            lst.append(it.key().c_str());
        }
        auto lstModel = new QStringListModel(lst);
        if(itr.key() == tables::tbDevicesType){
            ui->cmbDeviceType->setModel(lstModel);
            ui->cmbDeviceType->setCurrentText(result_.deviceType.c_str());
        }else if(itr.key() == tables::tbPriceTypes){
            ui->cmbTypePrice->setModel(lstModel);
            ui->cmbTypePrice->setCurrentText(key(result_.price_type, v));
        }else if(itr.key() == tables::tbOrganizations){
            ui->cmbOrganisation->setModel(lstModel);
            ui->cmbOrganisation->setCurrentText(key(result_.organization, v));
        }else if(itr.key() == tables::tbSubdivisions){
            ui->cmbSubdivisions->setModel(lstModel);
            ui->cmbSubdivisions->setCurrentText(key(result_.subdivision, v));
        }else if(itr.key() == tables::tbWarehouses){
            ui->cmbWarehouses->setModel(lstModel);
            ui->cmbWarehouses->setCurrentText(key(result_.warehouse, v));
        }else if(itr.key() == tables::tbWorkplaces){
            ui->cmbWorkplaces->setModel(lstModel);
            ui->cmbWorkplaces->setCurrentText(key(result_.workplace, v));
        }
    }

    ui->edtFirst->setText( result_.first.c_str());
    ui->edtSecond->setText( result_.second.c_str());
    ui->edtRef->setText( result_.ref.c_str());
    ui->txtAddress->setText(result_.address.c_str());

    if(ui->edtRef->text().toStdString() == arcirk::uuids::nil_string_uuid()){
        ui->editRef->setEnabled(true);
    }
}

DialogDevice::~DialogDevice()
{
    delete ui;
}

void DialogDevice::accept()
{
    result_.deviceType = ui->cmbDeviceType->currentText().toStdString();
    result_.price_type = tables_v[tables::tbPriceTypes][ui->cmbTypePrice->currentText().toStdString()];
    result_.organization = tables_v[tables::tbOrganizations][ui->cmbOrganisation->currentText().toStdString()];
    result_.subdivision = tables_v[tables::tbSubdivisions][ui->cmbSubdivisions->currentText().toStdString()];
    result_.warehouse = tables_v[tables::tbWarehouses][ui->cmbWarehouses->currentText().toStdString()];
    result_.workplace= tables_v[tables::tbWorkplaces][ui->cmbWorkplaces->currentText().toStdString()];
    result_.first = ui->edtFirst->text().toStdString();
    result_.second = ui->edtSecond->text().toStdString();
    result_.address = ui->txtAddress->text().toStdString();

    view_result_.ref = result_.ref;
    view_result_.first = result_.first;
    view_result_.second = result_.second;
    view_result_.device_type = result_.deviceType;
    view_result_.organization = ui->cmbOrganisation->currentText().toStdString();
    view_result_.subdivision = ui->cmbSubdivisions->currentText().toStdString();
    view_result_.price = ui->cmbTypePrice->currentText().toStdString();
    view_result_.warehouse = ui->cmbWarehouses->currentText().toStdString();
    view_result_.workplace = ui->cmbWorkplaces->currentText().toStdString();

    QDialog::accept();
}

arcirk::database::devices DialogDevice::get_result() const
{
    return result_;
}

arcirk::database::devices_view DialogDevice::get_view_result() const
{
    return view_result_;
}

QString DialogDevice::key(const std::string& value, QMap<std::string, std::string>& d) const
{
    for (auto itr = d.begin(); itr != d.end(); ++itr) {
        if(itr.value() == value)
            return QString::fromStdString(itr.key());
    }
    return {};
}



void DialogDevice::on_editRef_toggled(bool checked)
{
    ui->edtRef->setEnabled(checked);
    if(ui->edtRef->text().toStdString() == arcirk::uuids::nil_string_uuid()){
        ui->edtRef->setText(QUuid::createUuid().toString(QUuid::WithoutBraces));
        result_.ref = ui->edtRef->text().toStdString();
    }
}


void DialogDevice::on_edtRef_editingFinished()
{
    result_.ref = ui->editRef->text().toStdString();
}

