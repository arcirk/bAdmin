#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QUuid>
#include <nlohmann/json.hpp>
#include "shared_struct.hpp"
#include <QQueue>
#include <functional>

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

    bool isConnected();

    void open();
    void close();
    void write_conf();

    void send_command(arcirk::server::server_commands cmd, const nlohmann::json& param = {});

    nlohmann::json exec_http_query(const std::string& command, const nlohmann::json& param);

private:
    client::client_conf conf_;
    server::server_config server_conf;
    QWebSocket* m_client;

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

signals:
    void displayError(const QString& what, const QString& err);
    void connectionSuccess(); //при успешной авторизации
    void connectionChanged(bool state);
    void serverResponse(const arcirk::server::server_response& message);

private slots:

    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);
    void onReconnect();

};

#endif // WEBSOCKETCLIENT_H
