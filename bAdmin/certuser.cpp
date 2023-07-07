#include "certuser.h"
#include <QEventLoop>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include "cryptcertificate.h"
#include "commandline.h"
#include "commandlineparser.h"
#include <QSettings>
#include "websocketclient.h"

CertUser::CertUser(QObject *parent)
    : QObject{parent}
{

}

void CertUser::setLocalhost(bool value)
{
    is_localhost_ = value;

    if(is_localhost_){
        get_sid();
    }
}

QString CertUser::host()
{
    if(is_localhost_){
        return QSysInfo::machineHostName();
    }else{
        return {};
    }
}

QString CertUser::user_name()
{
    if(is_localhost_){
        if(!user_info_.user.empty()){
            auto lst = QString::fromStdString(user_info_.user).split('\\');
            if(lst.size() == 2)
                return lst[1];
            else
                return {};
        }
    }
    return {};
}

win_user_info CertUser::getInfo() const
{
    return user_info_;
}

nlohmann::json CertUser::getContainers()
{
    if(is_localhost_)
        return get_local_containers();
    else
        return {};
}

nlohmann::json CertUser::getCertificates(bool brief)
{
    if(is_localhost_)
        return get_local_certificates(brief);
    else
        return {};
}

QString CertUser::getCryptoProCSP() const
{
    QDir cryptoProDirectory;
    QString programFilesPath(QDir::fromNativeSeparators(getenv("PROGRAMFILES")));
    QString programFilesPath_x86 = programFilesPath;
    programFilesPath_x86.append(" (x86)");

    QDir x64(programFilesPath + "/Crypto Pro/CSP");
    QDir x86(programFilesPath_x86 + "/Crypto Pro/CSP");

    if(x86.exists()){
        QFile cryptcp(x86.path() + "/cryptcp.exe");
        if(cryptcp.exists()){
            cryptoProDirectory = x86;
        }
    }else
        if(x64.exists()){
            QFile cryptcp(x86.path() + "/cryptcp.exe");
            if(cryptcp.exists())
            cryptoProDirectory = x64;
        }


    return cryptoProDirectory.path();

}

void CertUser::get_sid()
{
    using json = nlohmann::json;

    auto cmd = CommandLine(this);
    QEventLoop loop;

    auto started = [&cmd]() -> void
    {
        cmd.send("WHOAMI /USER & exit", CmdCommand::wmicGetSID);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    json result{};
    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data) -> void
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

   std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp866");

   auto info = CommandLineParser::parse(result_.c_str(), wmicGetSID);

   try {
       user_info_ = pre::json::from_json<win_user_info>(info);
   } catch (const std::exception& e) {
       qCritical() << Q_FUNC_INFO << e.what();
   }

}

nlohmann::json CertUser::get_local_certificates(bool brief)
{

    using json = nlohmann::json;

    auto cryptoProLocation = getCryptoProCSP();

    if(cryptoProLocation.isEmpty())
        return {};

    auto cmd = CommandLine(this);
    cmd.setWorkingDirectory(cryptoProLocation);
    QEventLoop loop;

    auto started = [&cmd, &cryptoProLocation]() -> void
    {
        cmd.send(QString("certmgr -list -store uMy & exit").arg(cryptoProLocation), CmdCommand::csptestGetCertificates);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

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

    std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp866");


    auto info = CommandLineParser::parse(result_.c_str(), CmdCommand::csptestGetCertificates);

    auto table = json::object();
    auto tmp = arcirk::database::table_default_json(arcirk::database::tables::tbCertificates);
    auto items = tmp.items();
    auto columns = json::array({"first", "issuer", "subject", "private_key", "not_valid_before", "not_valid_after", "sha1", "suffix", "parent_user", "cache"});
    auto rows = json::array();

    if(!brief){
        columns.clear();
        for (auto itr = items.begin(); itr != items.end(); ++itr) {
            columns += itr.key();
        }
    }


    if(info.is_array()){
        if(info.size() > 0){
            for (auto itr = info.begin(); itr != info.end(); ++itr) {
                auto obj = *itr;
                auto cert_data = arcirk::database::table_default_struct<arcirk::database::certificates>(arcirk::database::tables::tbCertificates);
                CryptCertificate::load_response(cert_data, obj);
                if(brief){
                    auto row = json::object();
                    auto tmp_ = pre::json::to_json(cert_data);
                    for (auto it = columns.begin(); it != columns.end(); ++it) {
                        if(tmp_.find(*it) != tmp_.end()){
                            if(*it == "subject" || *it == "issuer"){

                            }
                            row[*it] = tmp_[*it];
                        }
                    }
                    rows += row;
                }else
                   rows += pre::json::to_json(cert_data);


            }
        }
        table["columns"] = columns;
        table["rows"] = rows;
    }

    return table;
}

nlohmann::json CertUser::get_local_containers()
{
    using json = nlohmann::json;

    auto cryptoProLocation = getCryptoProCSP();

    if(cryptoProLocation.isEmpty())
        return {};

    auto cmd = CommandLine(this);
    cmd.setWorkingDirectory(cryptoProLocation);

    QEventLoop loop;

    auto started = [&cmd]() -> void
    {
        cmd.send("csptest -keyset -enum_cont -fqcn -verifyc & exit", CmdCommand::csptestGetConteiners);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    json result{};
    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data) -> void
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

   std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp866");

   qDebug() << __FUNCTION__ << qPrintable(result_.data());

   auto info = CommandLineParser::parse(result_.c_str(), csptestGetConteiners);

   return info;
}

QStringList CertUser::read_mozilla_profiles()
{

    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    path.remove(APP_NAME);
    path.append("Mozilla/Firefox/profiles.ini");

    QDir::toNativeSeparators(QDir::homePath() + "/AppData/Roaming/Mozilla/Firefox/profiles.ini");

    QFile file(path);
    if(!file.exists())
        return {};

    QStringList result{};

    QSettings ini = QSettings(file.fileName(), QSettings::IniFormat);
    QStringList keys = ini.allKeys();
    foreach(const QString& key, keys){
        if(key.compare("Profile")){
            if(key.endsWith("/Name")){
                result.append(ini.value(key).toString());
            }
        }
    }

    return result;

}

void CertUser::set_database_cache(const std::string &data_str)
{
    try {
        data_ = json::parse(data_str);
    } catch (const std::exception& e) {
        data_ = json::object();
        qCritical() << e.what();
    }
}

void CertUser::read_database_cache(const QUrl &ws, const QString& token)
{
    json query_param = {
        {"table_name", arcirk::enum_synonym(tables::tbCertUsers)},
        {"query_type", "select"},
        {"values", json::array({"cache"})},
        {"where_values", json::object({
             {"host", host().toStdString()},
             {"system_user", user_name().toStdString()}
         })}
    };

    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto resp = WebSocketClient::http_query(ws, token, arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });

    data_ = json::object();

    try {
        if(resp.is_object()){
            auto rows = resp.value("rows", json::array());
            if(rows.size() != 0){
                data_ = json::parse(rows[0]["cache"].get<std::string>());
            }
        }
    } catch (const std::exception& e) {
        data_ = json::object();
        qCritical() << e.what();
    }

}

json CertUser::cache() const
{
    return data_;
}

json CertUser::get_container_info(const QString &name)
{
    using json = nlohmann::json;

    auto cryptoProLocation = getCryptoProCSP();

    if(cryptoProLocation.isEmpty())
        return {};

    auto cmd = CommandLine(this);
    cmd.setWorkingDirectory(cryptoProLocation);

    QEventLoop loop;

    auto started = [&cmd, name]() -> void
    {
        cmd.send(QString("csptest -keyset -container \"%1\" -info & exit").arg(name), CmdCommand::csptestContainerFnfo);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    json result{};
    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data) -> void
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

   std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp866");

   qDebug() << __FUNCTION__ << qPrintable(result_.data());

   auto info = CommandLineParser::parse(result_.c_str(), csptestContainerFnfo);

   return info;
}
