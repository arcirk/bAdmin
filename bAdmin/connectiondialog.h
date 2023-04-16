#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <shared_struct.hpp>
#include <QCryptographicHash>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(arcirk::client::client_conf& conf, QWidget *parent = nullptr);
    ~ConnectionDialog();

    void accept() override;
    bool connectionState() const;
    void setConnectionState(bool newConnectionState);
private slots:

    void on_toolButton_3_clicked();

    void on_btnPwdEdit_toggled(bool checked);

    void on_btnViewPwd_toggled(bool checked);

public slots:
    void connectionChanged(bool state);
private:
    Ui::ConnectionDialog *ui;
    arcirk::client::client_conf& conf_;
    bool m_connectionState;
    void formControl();
    QString get_sha1(const QByteArray& p_arg);
    QString get_hash(const QString& first, const QString& second);

signals:
    void openConnection();
    void closeConnection();
};

#endif // CONNECTIONDIALOG_H
