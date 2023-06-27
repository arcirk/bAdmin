#include "cryptcertificate.h"
#include "commandline.h"
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include "commandlineparser.h"
#include <QUrl>
#include <QTemporaryFile>

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

    QString path_(path);

    is_valid = false;

    QUrl url(path);

    if(!url.isLocalFile()){
        QFile f(path);
        if(f.open(QFile::ReadOnly)){
            QByteArray ba = f.readAll();
            f.close();
            auto tmp = new QTemporaryFile();
            tmp->setAutoRemove(false);
            tmp->open();
            tmp->write(ba);
            path_ = tmp->fileName();
            tmp->close();
            delete tmp;
        }
    }

    QFileInfo inf(path);
    QString suffix = inf.completeSuffix();

    auto cmd = CommandLine(this);
    QString cryptoPrp = cryptoProDirectory.path();
    cmd.setWorkingDirectory(cryptoPrp);

    QEventLoop loop;

    auto started = [&cmd, &path_]() -> void
    {
        QString s = QString("certutil \"%1\" & exit").arg(QDir::toNativeSeparators(path_));
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

    //std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp866");
    std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp1251");
    qDebug() << qPrintable(result_.c_str());
    result = CommandLineParser::parse(result_.c_str(), CmdCommand::certutilGetCertificateInfo);

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
    cert_info_.cache = result.dump();

    try {
        arcirk::read_file(QTextCodec::codecForName("CP1251")->fromUnicode(path_).toStdString(), cert_info_.data);
    } catch (const std::exception& e) {
        qCritical() << e.what();
    }

//    if(tmp.isWritable()){
//        tmp.remove();
//    }
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
    result.cache = object.dump();
}

bool CryptCertificate::save_as(const QString &sha1, const QString &file, QObject* parent)
{
    using json = nlohmann::json;

    auto cmd = CommandLine(parent);
    QString cryptoPro = get_crypto_pro_dir();
    Q_ASSERT(!cryptoPro.isEmpty());
    cmd.setWorkingDirectory(cryptoPro);

    QEventLoop loop;

    auto started = [&cmd, &file, &sha1]() -> void
    {
        QString command = QString("cryptcp -copycert -thumbprint \"%1\" -u -df \"%2\" & exit").arg(sha1, file);
        cmd.send(command, CmdCommand::certmgrExportlCert);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    //json result{};
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

   qDebug() << qPrintable(result_.c_str());

   //auto info__ = CommandLineParser::parse(result_.c_str(), certmgrExportlCert);

   //auto result = parse(info__.get<std::string>().c_str());

   return true;
}

QString CryptCertificate::get_crypto_pro_dir()
{
    bool is_found = false;
    QString programFilesPath(QDir::fromNativeSeparators(getenv("PROGRAMFILES")));
    QString programFilesPath_x86 = programFilesPath;
    programFilesPath_x86.append(" (x86)");

    QDir x64(programFilesPath + "/Crypto Pro/CSP");
    QDir x86(programFilesPath_x86 + "/Crypto Pro/CSP");

    QDir result;

    if(x86.exists()){
        QFile cryptcp(x86.path() + "/cryptcp.exe");
        if(cryptcp.exists()){
            is_found = true;
            result = x86;
        }
    }else
        if(x64.exists()){
            QFile cryptcp(x86.path() + "/cryptcp.exe");
            if(cryptcp.exists())
                is_found = true;
            result = x64;
        }


    if(!is_found)
       return "";
    else
        return result.path();
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
