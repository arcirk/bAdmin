#ifndef CERTUSER_H
#define CERTUSER_H

#include <QObject>
#include "shared_struct.hpp"

using namespace arcirk::cryptography;
using namespace arcirk::database;
using json = nlohmann::json;

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
    //static win_user_info get_win_user_info();

    json getCertificates(bool brief);

    json getContainers();

    QString getCryptoProCSP() const;

    static QStringList read_mozilla_profiles();

    void set_database_cache(const std::string& data_str);

    void read_database_cache(const QUrl &ws, const QString& token);

    json cache() const;

    json get_container_info(const QString& name);

private:
    win_user_info user_info_;
    cert_users cert_user_;
    json data_;

    bool is_valid_;
    bool is_localhost_;

    void get_sid();

    json get_local_certificates(bool brief);
    json get_local_containers();



signals:

};

#endif // CERTUSER_H
