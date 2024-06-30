#ifndef RESOURCESSERVER_H
#define RESOURCESSERVER_H

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "IResourcesServer.h"
#include "ResourcesServer_global.h"

class RESOURCESSERVER_EXPORT ResourcesServer : public QObject, public IResourcesServer
{
    Q_OBJECT
    Q_INTERFACES(IResourcesServer)

public:
    ResourcesServer(QObject* parent);
    ~ResourcesServer();

    void setPort(int port) override;

    int port() const override;

    void setClientLimit(int limit) override;

    int clientLimit() const;

    void setPermittedUsers(const QStringList& usernames) override;

    QStringList permittedUsers() const override;

    void setZTime(int ms) override;

    int zTime() const override;

    void run() override;

    bool isRunning() const override;

    void freeResources() override;

    void rejectAuthRequests(bool reject) override;

    void rejectRcRequests(bool reject) override;

    /// @brief Остановка сервера
    void stop();

    const QMap<QTcpSocket*, QString>& clients() const;

    QMap<int, QString> resources() const;

    void loadSettings();

    QMap<QString, QTime> resourceTimes() const;

private slots:
    void onReadyRead();
    void onDisconnected();
    void onNewConnection();

signals:
    void usersUpdated();
    void resourcesUpdated();

private:
    bool isPermittedUser(const QString& username);
    void handleAuthRequest(QTcpSocket* client, const QJsonObject& json);
    void handleResourceRequest(QTcpSocket* client, const QJsonObject& json);
    void sendResponse(QTcpSocket* client, const QString& username, int resource, int status);

private:
    QTcpServer*                _tcpServer;
    int                        _port;
    int                        _clientLimit;
    QStringList                _permittedUsers;
    int                        _zTime; ///< Время на которое ресурс будет точно занят
    bool                       _running;
    bool                       _rejectAuth;
    bool                       _rejectRc;
    QMap<QTcpSocket*, QString> _clients;
    QMap<int, QString>         _resources;
    QMap<int, QTimer*>         _resourcesTimers;
    QTcpSocket*                _client;
    QStringList                _bannedUsers;
    QMap<QString, int>         _time;
    QMap<QString, QTime>       _resourceTimes; ///< Каждому юзеру присваиваем время когда он занял ресурсы
};

#endif // RESOURCESSERVER_H
