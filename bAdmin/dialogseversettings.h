#ifndef DIALOGSEVERSETTINGS_H
#define DIALOGSEVERSETTINGS_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include "shared_struct.hpp"

namespace Ui {
class DialogSeverSettings;
}

class DialogSeverSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSeverSettings(arcirk::server::server_config& conf, arcirk::client::client_conf& conf_cl, QWidget *parent = nullptr);
    ~DialogSeverSettings();

    void accept() override;

    arcirk::server::server_config getResult();

private slots:
    void on_btnEditHSPassword_toggled(bool checked);

    void on_btnViewHSPassword_toggled(bool checked);

    void on_btnEditWebDavPwd_toggled(bool checked);

    void on_btnViewWebDavPwd_toggled(bool checked);

    void on_btnEditSQLPassword_toggled(bool checked);

    void on_btnViewSQLPassword_toggled(bool checked);

    void on_btnSelPriceCheckerRepo_clicked();

private:
    Ui::DialogSeverSettings *ui;
    arcirk::server::server_config& conf_;
    arcirk::client::client_conf& conf_cl_;

    void onPwdEditToggled(bool checked, QToolButton* sender);
    void onViewPwdToggled(bool checked, QToolButton* sender, QLineEdit* pwd);
};

#endif // DIALOGSEVERSETTINGS_H
