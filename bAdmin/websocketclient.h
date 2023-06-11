#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QUuid>
#include <nlohmann/json.hpp>
#include "shared_struct.hpp"
#include <QQueue>
#include <functional>
#include <QTimer>

using namespace arcirk;

typedef std::function<void()> async_await;

class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    explicit WebSocketClient(const QUrl& url, QObject *parent = nullptr);
    ~WebSocketClient();

    static QString generateHash(const QString& usr, const QString& pwd);

    bool isStarted();

    client::client_conf& conf();
    server::server_config& server_conf();

    bool isConnected();

    QUrl url() const;

    void open();
    void close();
    void write_conf();

    void send_command(arcirk::server::server_commands cmd, const nlohmann::json& param = {});
    void command_to_client(const std::string &receiver, const std::string &command,
                                         const nlohmann::json &param = {});

    nlohmann::json exec_http_query(const std::string& command, const nlohmann::json& param, const ByteArray& data = {});
    static nlohmann::json http_query(const QUrl& ws, const QString& token, const std::string& command, const nlohmann::json& param, const ByteArray& data = {});

    QByteArray exec_http_query_get(const std::string& command, const nlohmann::json& param);

    static std::string crypt(const QString &source, const QString &key);

    static arcirk::client::version_application get_version(){
         QStringList vec = QString(ARCIRK_VERSION).split(".");
         auto ver = arcirk::client::version_application();
         ver.major = vec[0].toInt();
         ver.minor = vec[1].toInt();
         ver.path = vec[2].toInt();
        return ver;
    }

    void register_device(const arcirk::client::session_info& sess_info);

    void set_system_user(const QString& value);

    QUuid currentSession() const;

private:
    client::client_conf conf_;
    server::server_config server_conf_;
    QWebSocket* m_client;
    QString system_user_;

    QUuid m_currentSession;
    QUuid m_currentUserUuid;
    QTimer * m_reconnect;

    QQueue<async_await> m_async_await;

    bool m_isConnected;

    void read_conf();

    void initialize();

    void parse_response(const QString& resp);

    void asyncAwait(){
        if(m_async_await.size() > 0){
            auto f = m_async_await.dequeue();
            f();
        }
    };

    void doDisplayError(const QString& what, const QString& err);
    void doConnectionSuccess(); //при успешной авторизации
    void doConnectionChanged(bool state);

    static QString get_hash(const QString& first, const QString& second);
    static QString get_sha1(const QByteArray& p_arg);


    void get_server_configuration_sync();

    static arcirk::client::client_param parse_client_param(const std::string& response);

signals:
    void displayError(const QString& what, const QString& err);
    void connectionSuccess(); //при успешной авторизации
    void connectionChanged(bool state);
    void serverResponse(const arcirk::server::server_response& message);
    void notify(const QString& message);

private slots:

    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error, const QString& errorString);
    void onReconnect();

};

#endif // WEBSOCKETCLIENT_H
