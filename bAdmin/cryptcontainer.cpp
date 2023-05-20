#include "cryptcontainer.h"
#include <QEventLoop>
#include <QDebug>

CryptContainer::CryptContainer(QObject *parent)
    : QObject{parent}
{
    is_valid = false;

    get_sid();
}

QString CryptContainer::sid() const
{
    return sid_;
}

void CryptContainer::get_sid()
{
    using json = nlohmann::json;

    auto cmd = CommandLine(this);
    QEventLoop loop;

    auto started = [&cmd]() -> void
    {
        cmd.send("WHOAMI /USER ; exit", CmdCommand::wmicGetSID);
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

    auto codec = QTextCodec::codecForName("Windows-1251");
    //QTextCodec::setCodecForLocale(codec);
    //qDebug() << QString::fromLocal8Bit(cmd_text);
    qDebug() << qPrintable(codec->toUnicode(cmd_text));
    //qDebug() << cmd_text;
}
