#include "cryptcertificate.h"
#include "commandline.h"
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include "commandlineparser.h"

bool CryptCertificate::isValid()
{
    return is_valid;
}

CryptCertificate::CryptCertificate(QObject *parent)
    : QObject{parent}
{
    is_valid = false;

    //auto program_dir = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    bool is_found = false;
    QString programFilesPath(QDir::fromNativeSeparators(getenv("PROGRAMFILES")));
    QString programFilesPath_x86 = programFilesPath;
    programFilesPath_x86.append(" (x86)");

    QDir x64(programFilesPath + "/Crypto Pro/CSP");
    QDir x86(programFilesPath_x86 + "/Crypto Pro/CSP");

    if(x86.exists()){
        QFile cryptcp(x86.path() + "/cryptcp.exe");
        if(cryptcp.exists()){
            is_found = true;
            cryptoProDirectory = x86;
        }
    }else
        if(x64.exists()){
            QFile cryptcp(x86.path() + "/cryptcp.exe");
            if(cryptcp.exists())
                is_found = true;
            cryptoProDirectory = x64;
        }


    if(!is_found)
        emit error("CryptCertificate::CryptCertificate", "КриптоПро не найден!");
}

void CryptCertificate::fromLocal(const QString &sha)
{
    is_valid = false;

}

bool CryptCertificate::fromFile(const QString &path)
{
    using json = nlohmann::json;

    is_valid = false;

    QFileInfo inf(path);
    QString suffix = inf.completeSuffix();

    auto cmd = CommandLine(this);

    QEventLoop loop;

    QString cryptoPrp = cryptoProDirectory.path();

    auto codec = QTextCodec::codecForName("CP1251");
    QTextCodec::setCodecForLocale(codec);

    auto started = [&cmd, &path, &cryptoPrp]() -> void
    {
        QString s = QString("cd %1 & certutil \"%2\" & exit").arg(cryptoPrp, path);
        cmd.send(s, CmdCommand::certutilGetCertificateInfo);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    //QString str;
    json result{};
    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data, CmdCommand command) -> void
    {
        cmd_text.append(data);
    };
    loop.connect(&cmd, &CommandLine::output, output);

    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        qDebug() << __FUNCTION__ << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);


    cmd.start();
    loop.exec();

    result = CommandLineParser::parse(codec->toUnicode(cmd_text), CmdCommand::certutilGetCertificateInfo);

    if(result.empty() || !result.is_object())
        return false;



    auto st = pre::json::to_json(cert_info());

    for (auto itr = result.items().begin(); itr != result.items().end(); ++itr) {
        if(st.find(itr.key()) != st.end()){
            st[itr.key()] = itr.value();
        }
    }

    cert_info_ = pre::json::from_json<cert_info>(st);
    cert_info_.suffix = suffix.toStdString();

    try {
        arcirk::read_file(QTextCodec::codecForName("CP1251")->fromUnicode(path).toStdString(), cert_info_.data);
    } catch (const std::exception& e) {
        qCritical() << e.what();
    }

    is_valid = true;

    return true;

}

cert_info CryptCertificate::getData() const
{
    return cert_info_;
}

nlohmann::json CryptCertificate::issuer() const
{
    if(!is_valid)
        return {};

    return parse_details(cert_info_.issuer);
}

nlohmann::json CryptCertificate::subject() const
{
    if(!is_valid)
        return {};

    return parse_details(cert_info_.subject);
}

std::string CryptCertificate::issuer_briefly() const
{
    auto v = issuer();

    return v.value("CN", "");
}

std::string CryptCertificate::subject_briefly() const
{
    auto v = subject();

    return v.value("CN", "");
}

std::string CryptCertificate::sinonym() const
{
    if(!is_valid)
        return "";

    auto subject_ = subject();

    return subject_.value("CN", "") + " " + cert_info_.not_valid_before + " - " + cert_info_.not_valid_after;
}

std::string CryptCertificate::sha1() const
{
    if(!is_valid)
        return "";

    return cert_info_.sha1;
}

std::string CryptCertificate::parent() const
{
    if(!is_valid)
        return "";

    auto sub = subject();

    auto SN = sub.value("SN", "");
    auto G = sub.value("G", "");

    return SN + " " + G;
}

std::string CryptCertificate::dump() const
{
    return pre::json::to_json(cert_struct).dump();
}

void CryptCertificate::load_response(arcirk::database::certificates& result, const nlohmann::json& object)
{
    //result.first = object.value()
    result.issuer = object.value("Issuer", "");
    if(!result.issuer.empty()){
        QString issuer(result.issuer.c_str());
        auto lst = issuer.split(",");
        QMap<QString,QString> m_lst;
        foreach(auto str, lst){
            if(str.indexOf("=") !=-1){
                auto ind = str.split("=");
                auto second = ind[1].trimmed();
                if(second[0] == '"'){
                    //second = second.mid(1, second.length() - 1);
                    second.remove('\"');
                }
                m_lst.insert(ind[0].trimmed(), second);
            }
        }

        result.first = m_lst["CN"].toStdString();
    }
    result.subject = object.value("Subject", "");
    if(!result.issuer.empty()){
        QString issuer(result.issuer.c_str());
        auto lst = issuer.split(",");
        QMap<QString,QString> m_lst;
        foreach(auto str, lst){
            if(str.indexOf("=") !=-1){
                auto ind = str.split("=");
                m_lst.insert(ind[0].trimmed(), ind[1].trimmed());
            }
        }
        result.second = m_lst["CN"].toStdString();
        result.parent_user = m_lst["SN"].toStdString() + " " + m_lst["G"].toStdString();
    }
    result.private_key = object.value("Container", "");
    result.not_valid_before = object.value("Not valid before", "");
    result.not_valid_after = object.value("Not valid after", "");
    result.serial = object.value("Serial", "");
    result.sha1 = object.value("SHA1 Hash", "");
}

nlohmann::json CryptCertificate::parse_details(const std::string &details) const
{
    auto s = QString::fromStdString(details);
    auto lst = s.split("\r ");
    auto result = nlohmann::json::object();
    foreach (auto val, lst) {
        auto ind = val.indexOf("=");
        if(ind != -1){
            auto key = val.left(ind).trimmed();
            auto value = val.right(val.length() - ind - 1).trimmed();
            result[key.toStdString()] = value.toStdString();
        }
    }

    return result;
}
