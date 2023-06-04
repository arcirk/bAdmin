#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include "shared_struct.hpp"
#include <QProcess>
#include <QTextCodec>

#define DEFAULT_CHARSET_ "CP866"
#define DEFAULT_CHARSET_WIN "CP1251"

using namespace  arcirk::command_line;

class CommandLine : public QObject
{
    Q_OBJECT
public:
    explicit CommandLine(QObject *parent = nullptr);
    bool listening();
    void setWorkingDirectory(const QString& value);

public slots:
    void start();
    void stop();
    void send(const QString& commandText, CmdCommand cmd);
    void onParse(const QVariant& result, CmdCommand cmd);

signals:
    void output(const QByteArray& data, CmdCommand command);
    void error(const QString& data, CmdCommand command);
    void complete();
    void parse(const QVariant& result, CmdCommand command);
    void started_process();

private slots:
    void errorOccured(QProcess::ProcessError err);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void readyReadStandardError();
    void readyReadStandardOutput();
    void started();
    void stateChanged(QProcess::ProcessState newState);
    void readyRead();

private:
    QProcess m_process;
    bool m_listening;
    CmdCommand m_command;
    QString _strPart;
    QString program;
    QString _lastCommand;
    QTextCodec *codec;
    QTextCodec *codec866;
};

#endif // COMMANDLINE_H
