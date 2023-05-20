#include "commandlineparser.h"
#include <QDebug>

CommandLineParser::CommandLineParser(QObject *parent)
    : QObject{parent}
{

}

nlohmann::json CommandLineParser::parse(const QString &response, CmdCommand command)
{
    using json = nlohmann::json;

    try {
        if(command == certutilGetCertificateInfo){
            int ind = response.indexOf("CertUtil: -dump");
            if(ind == -1)
                return {};

            QString line = getLine(response, ind);

            if(!line.isEmpty()){
                QStringList r = line.split(":");
                if(r.size() == 2){
                    QString s = r[1].trimmed();
                    if(s == "-dump — команда успешно выполнена."){
                        //auto arr = json::array();
                        auto obj = json::object();
                        ind = response.indexOf("Серийный номер:");
                        if(ind > 0){
                            QString sz = getLine(response, ind);
                            //sz.replace("Серийный номер", "Serial");
                            auto s_r = sz.split(":");
                            obj["serial"] = s_r[1].trimmed().toStdString();
                        }
                        ind = response.indexOf("Поставщик:");
                        if(ind > 0){
                            int ind_ = response.indexOf("Хэш имени (md5)", ind);
                            if(ind_ != -1){
                                QString sz_ = getLine(response, ind_);
                                int ind__ = response.lastIndexOf(sz_);
                                if(ind__ != -1){
                                    QString Issuer = response.mid(ind, ind__ - ind);
                                    Issuer.replace("\n", "");
                                    auto s_r = Issuer.split(":");
                                    obj["issuer"] = s_r[1].trimmed().toStdString();
                                }
                            }
                            ind = response.indexOf("NotBefore:");
                            if(ind > 0){
                                auto dt = getLine(response, ind).trimmed();
                                obj["not_valid_before"] = dt.right(dt.length() - QString("NotBefore:").length()).trimmed().toStdString();
                                //dt = dt.mid(dt.lastIndexOf("NotBefore:"));
                                //auto s_r = getLine(response, ind).trimmed().split(":");
                                //obj["not_valid_before"] = s_r[1].trimmed().toStdString();
                            }
                            ind = response.indexOf("NotAfter:");
                            if(ind > 0){
//                                auto s_r = getLine(response, ind).trimmed().split(":");
//                                obj["not_valid_after"] = s_r[1].trimmed().toStdString();
                                auto dt = getLine(response, ind).trimmed();
                                obj["not_valid_after"] = dt.right(dt.length() - QString("NotAfter:").length()).trimmed().toStdString();
                            }
                            ind = response.indexOf("Субъект:");
                            if(ind > 0){
                                int ind_ = response.indexOf("Хэш имени (md5)", ind);
                                if(ind_ != -1){
                                    QString sz_ = getLine(response, ind_);
                                    int ind__ = response.lastIndexOf(sz_);
                                    if(ind__ != -1){
                                        QString subject = response.mid(ind, ind__ - ind);
                                        subject.replace("\n", "");
                                        auto s_r = subject.split(":");
                                        obj["subject"] = s_r[1].trimmed().toStdString();
                                    }
                                }
                            }
                            ind = response.indexOf("Хеш сертификата(sha1):");
                            if(ind > 0){
                                auto s_r = getLine(response, ind).trimmed().split(":");
                                obj["sha1"] = s_r[1].trimmed().toStdString();
                            }
                            //arr += obj;
                            return obj;
                        }
                    }
                }
            }
        }else if(command == wmicGetSID){
//            QString str(result);
//            QRegularExpression  re( "S-1");
//            int length = QString("S-1").length();
//            int l = result.lastIndexOf(re, length);
//            int in = result.indexOf(re, length);
//            int fwd = l > in ? l : in;
//            if(fwd >= 0){
//                for(int i = fwd; i < str.length(); ++i){
//                    QString s = str.mid(i, 1);
//                    if(s == " " || s == "\n" || s == "\r"){
//                        QString _res = str.mid(fwd, i - fwd);
//                        emit endParse(_res, command);
//                        break;
//                    }
//                }
//            }
        }

    } catch (const std::exception& e) {

        qCritical() << e.what();
    }


    return {};
}


QString CommandLineParser::getLine(const QString &source, int start)
{
    for (int i = start; i < source.length(); ++i) {
        QString s = source.mid(i, 1);
        if(s == "\n" || s == "\r" || i == source.length() - 1)
            return source.mid(start, i - start);
    }

    return "";
}
