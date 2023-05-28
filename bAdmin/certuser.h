#ifndef CERTUSER_H
#define CERTUSER_H

#include <QObject>
#include "shared_struct.hpp"

using namespace arcirk::cryptography;
using namespace arcirk::database;

class CertUser : public QObject
{
    Q_OBJECT
public:
    explicit CertUser(QObject *parent = nullptr);

    void setLocalhost(bool value);
    void isValid();

    QString host();
    QString user_name();

    win_user_info getInfo() const;

    nlohmann::json getCertificates();

    nlohmann::json getContainers();

    QString getCryptoProCSP() const;


private:
    win_user_info user_info_;
    cert_users cert_user_;
    bool is_valid_;
    bool is_localhost_;

    void get_sid();

    nlohmann::json get_local_certificates();
    nlohmann::json get_local_containers();

signals:

};

#endif // CERTUSER_H
