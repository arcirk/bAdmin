#ifndef CRYPTCERTIFICATE_H
#define CRYPTCERTIFICATE_H

#include <QObject>
#include "shared_struct.hpp"
#include <QDir>

using namespace arcirk::cryptography;

class CryptCertificate : public QObject
{
    Q_OBJECT
public:
    bool isValid();

    explicit CryptCertificate(QObject *parent = nullptr);

    bool fromLocal(const QString &sha);
    bool fromFile(const QString &path);
    bool fromByteArray(const QByteArray& data);

    bool install(const QString& container = "", QObject *parent = nullptr);
    void remove(const QString &sha1, QObject *parent = nullptr);

    cert_info getData() const;

    nlohmann::json issuer() const;
    nlohmann::json subject() const;
    std::string issuer_briefly() const;
    std::string subject_briefly() const;
    std::string sinonym() const;
    std::string sha1() const;
    std::string parent() const;
    std::string dump() const;

    static void load_response(arcirk::database::certificates& result, const nlohmann::json& object);

    static bool save_as(const QString& sha1, const QString& file, QObject *parent);
    static QString get_crypto_pro_dir();


private:
    arcirk::database::certificates cert_struct;
    bool is_valid;
    QDir cryptoProDirectory;
    cert_info cert_info_;

    nlohmann::json parse_details(const std::string& details) const;

signals:
    void error(const QString& what, const QString& err);
};

#endif // CRYPTCERTIFICATE_H
