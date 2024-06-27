#ifndef RESOURCESSERVER_H
#define RESOURCESSERVER_H

#include <QTcpServer>
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

    void        setPort(int port) override;
    int         port() const override;
    void        setClientLimit(int limit) override;
    void        setPermittedUsers(const QStringList& usernames) override;
    QStringList permittedUsers() const override;
    void        setZTime(int ms) override;
    int         zTime() const override;
    void        run() override;
    bool        isRunning() const override;
    void        freeResources() override;
    void        rejectAuthRequests(bool reject) override;
    void        rejectRcRequests(bool reject) override;

    // protected:
    //     void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    bool isPermittedUser(const QString& username);
    void handleAuthRequest(QTcpSocket* client, const QJsonObject& json);
    void handleResourceRequest(QTcpSocket* client, const QJsonObject& json);
    void sendResponse(QTcpSocket* client, const QString& username, int resource, int status);

private:
    int                        _port;
    int                        _clientLimit;
    QStringList                _permittedUsers;
    int                        _zTime;
    bool                       _running;
    bool                       _rejectAuth;
    bool                       _rejectRc;
    QMap<QTcpSocket*, QString> _clients;
    QMap<int, QString>         _resources;
    QMap<int, QTimer*>         _resourcesTimers;
    QTcpServer*                _tcpServer;
};

#endif // RESOURCESSERVER_H
