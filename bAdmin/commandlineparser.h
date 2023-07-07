#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QObject>
#include "shared_struct.hpp"

using namespace arcirk::command_line;

class CommandLineParser : public QObject
{
    Q_OBJECT
public:
    CommandLineParser(QObject *parent = nullptr);

    static nlohmann::json parse(const QString& response, CmdCommand command);
    static nlohmann::json parse_details(const std::string &details);
    static QString getLine(const QString &source, int start);

};

#endif // COMMANDLINEPARSER_H
