#include "ResourcesServer.h"

ResourcesServer::ResourcesServer(QObject* parent)
    : _port(0)
    , _clientLimit(IResourcesServer::ABSOLUTE_CLINET_LIMIT)
    , _zTime(30000)
    , _running(false)
    , _rejectAuth(false)
    , _rejectRc(false)
{
    _tcpServer = new QTcpServer(this);
}

ResourcesServer::~ResourcesServer()
{
}

void ResourcesServer::setPort(int port)
{
    _port = port;
}

int ResourcesServer::port() const
{
    return _port;
}

void ResourcesServer::setClientLimit(int limit)
{
    if (_clientLimit > IResourcesServer::ABSOLUTE_CLINET_LIMIT)
    {
        _clientLimit = IResourcesServer::ABSOLUTE_CLINET_LIMIT;
    }
    else
    {
        _clientLimit = limit;
    }
}

void ResourcesServer::setPermittedUsers(const QStringList& usernames)
{
    _permittedUsers = usernames;
}

QStringList ResourcesServer::permittedUsers() const
{
    return _permittedUsers;
}

void ResourcesServer::setZTime(int ms)
{
    _zTime = ms;
}

int ResourcesServer::zTime() const
{
    return _zTime;
}

void ResourcesServer::run()
{
}

bool ResourcesServer::isRunning() const
{
    return _running;
}

void ResourcesServer::freeResources()
{
}

void ResourcesServer::rejectAuthRequests(bool reject)
{
}

void ResourcesServer::rejectRcRequests(bool reject)
{
}

bool ResourcesServer::isPermittedUser(const QString& username)
{
    return _permittedUsers.contains(username);
}

void ResourcesServer::handleAuthRequest(QTcpSocket* client, const QJsonObject& json)
{
}

void ResourcesServer::handleResourceRequest(QTcpSocket* client, const QJsonObject& json)
{
}

void ResourcesServer::sendResponse(QTcpSocket* client, const QString& username, int resource, int status)
{
}
