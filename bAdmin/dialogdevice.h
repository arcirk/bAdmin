#ifndef DIALOGDEVICE_H
#define DIALOGDEVICE_H

#include <QDialog>
#include "shared_struct.hpp"
#include <QMap>

using namespace arcirk::database;
using json = nlohmann::json;

typedef QMap<tables, QMap<std::string, std::string>> Tables_v;
namespace Ui {
class DialogDevice;
}

class DialogDevice : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDevice(const nlohmann::json& deviceDetails, QWidget *parent = nullptr);
    ~DialogDevice();

    void accept() override;

    arcirk::database::devices get_result() const;
    arcirk::database::devices_view get_view_result() const;

private slots:


    void on_editRef_toggled(bool checked);

    void on_edtRef_editingFinished();

private:
    Ui::DialogDevice *ui;
    arcirk::database::devices result_;
    arcirk::database::devices_view view_result_;
    Tables_v tables_v{};

    QString key(const std::string& value, QMap<std::string, std::string>& d) const;
};

#endif // DIALOGDEVICE_H
