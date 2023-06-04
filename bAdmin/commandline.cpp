#include "commandline.h"
#include <QDebug>

CommandLine::CommandLine(QObject *parent)
    : QObject{parent}
{
    connect(&m_process, &QProcess::errorOccurred, this, &CommandLine::errorOccured);
    connect(&m_process, &QProcess::readyReadStandardError, this, &CommandLine::readyReadStandardError);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &CommandLine::readyReadStandardOutput);
    connect(&m_process, &QProcess::started, this, &CommandLine::start);
    connect(&m_process, &QProcess::stateChanged, this, &CommandLine::stateChanged);
    connect(&m_process, &QProcess::readyRead, this, &CommandLine::readyRead);

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CommandLine::finished);

    m_listening = false;
    m_command = COMMAND_INVALID;
    program = "cmd"; //"powershell";

    codec866 = QTextCodec::codecForName(DEFAULT_CHARSET_);
    codec = QTextCodec::codecForName(DEFAULT_CHARSET_WIN);
    QTextCodec::setCodecForLocale(codec);
}

bool CommandLine::listening()
{
    return m_listening;
}

void CommandLine::setWorkingDirectory(const QString &value)
{
    m_process.setWorkingDirectory(value);
}

void CommandLine::start()
{
    if(m_listening)
        return;

    m_listening = true;
    m_process.start(program);
}

void CommandLine::stop()
{
    m_listening = false;
    if(m_process.state() == QProcess::Running)
        m_process.close();
}

void CommandLine::send(const QString &commandText, CmdCommand cmd)
{
    _strPart = "";
    QString _commandText = commandText;
    if(QString(commandText).right(1) != "\n")
        _commandText = commandText + "\n";

    if(program == "powershell"){
        _commandText.replace("\"", "'");
        if(_commandText.left(QString("csptest").length()) == "csptest"){
            _commandText = ".\\" + _commandText;
        }
    }

    _lastCommand = _commandText;

    m_command = cmd;
    if(m_listening)
        m_process.write(codec866->fromUnicode(_commandText));

}

void CommandLine::onParse(const QVariant &result, CmdCommand cmd)
{
    emit parse(result, cmd);
}

void CommandLine::errorOccured(QProcess::ProcessError err)
{
    if(!m_listening) return;

    qInfo() << Q_FUNC_INFO << err;
    emit error("Error", m_command);
}

void CommandLine::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(!m_listening) return;
    //qInfo() << Q_FUNC_INFO;
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    emit output("Complete", COMMAND_INVALID);
}

void CommandLine::readyReadStandardError()
{
    if(!m_listening) return;

    QByteArray data = m_process.readAllStandardError();
    emit error(data, m_command);
}

void CommandLine::readyReadStandardOutput()
{

}

void CommandLine::started()
{
    qInfo() << Q_FUNC_INFO;
}

void CommandLine::stateChanged(QProcess::ProcessState newState)
{
    qInfo() << Q_FUNC_INFO << newState;
    switch (newState) {
        case QProcess::NotRunning:
            emit output("Not running", COMMAND_INVALID);
            emit complete();
            break;
        case QProcess::Starting:
            emit output("Starting ...", COMMAND_INVALID);
            break;
        case QProcess::Running:{
            emit output("Running", COMMAND_INVALID);
            emit started_process();
        }
            break;
    }
}

void CommandLine::readyRead()
{
    if(!m_listening) return;

    QByteArray data = m_process.readAll();
    emit output(data, m_command);
}
