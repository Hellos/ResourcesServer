#include "ResourcesServer.h"

ResourcesServer::ResourcesServer(QObject* parent)
    : _tcpServer(new QTcpServer(this))
    , _port(0)
    , _clientLimit(ResourcesServer::ABSOLUTE_CLINET_LIMIT)
    , _zTime(30000)
    , _running(false)
    , _rejectAuth(false)
    , _rejectRc(false)
{
    connect(_tcpServer, &QTcpServer::newConnection, this, &ResourcesServer::onNewConnection);

    for (int i = 0; i < ResourcesServer::RESOURCE_QUANTITY; i++)
    {
        _resources[i]       = "";
        _resourcesTimers[i] = new QTimer(this);
    }

    loadSettings();
}

ResourcesServer::~ResourcesServer()
{
    stop();
}

void ResourcesServer::setPort(int port)
{
    if (port < 0 || port > 65535)
    {
        _port = 0;
    }
    else
    {
        _port = port;
    }
}

int ResourcesServer::port() const
{
    return _port;
}

void ResourcesServer::setClientLimit(int limit)
{
    if (_clientLimit > ResourcesServer::ABSOLUTE_CLINET_LIMIT)
    {
        _clientLimit = ResourcesServer::ABSOLUTE_CLINET_LIMIT;
    }
    else
    {
        _clientLimit = limit;
    }
}

int ResourcesServer::clientLimit() const
{
    return _clientLimit;
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
    if (_zTime < 0)
    {
        _zTime = 30;
    }
    else
    {
        _zTime = ms;
    }
}

int ResourcesServer::zTime() const
{
    return _zTime;
}

void ResourcesServer::run()
{
    if (_tcpServer->listen(QHostAddress::Any, _port))
    {
        _running = true;
        qInfo() << "Сервер запущен на порту:" << _tcpServer->serverPort();
    }
}

bool ResourcesServer::isRunning() const
{
    return _running;
}

void ResourcesServer::freeResources()
{
    for (int i = 0; i < ResourcesServer::RESOURCE_QUANTITY; i++)
    {
        QTcpSocket* tmp = _clients.key(_resources[i], nullptr);
        if (!(_resources[i] == "") && tmp && tmp->isValid())
        {
            sendResponse(_clients.key(_resources[i]), _resources[i], i + 1, 2);
            qInfo() << "Ресурс" << i + 1 << "был освобождён";
        }
        _resources[i] = "";
        _resourcesTimers[i]->stop();
    }
    _resourceTimes.clear();
    emit resourcesUpdated();
}

void ResourcesServer::rejectAuthRequests(bool reject)
{
    _rejectAuth = reject;
}

void ResourcesServer::rejectRcRequests(bool reject)
{
    _rejectRc = reject;
}

void ResourcesServer::stop()
{
    qInfo() << "Сервер выключен";
    _running = false;
    for (auto& cl : _clients.keys())
    {
        cl->disconnectFromHost();
        cl->deleteLater();
        _clients.remove(cl);
    }
    freeResources();
    _bannedUsers.clear();
    _tcpServer->close();
}

void ResourcesServer::onReadyRead()
{
    _client = qobject_cast<QTcpSocket*>(sender());
    if (!_client)
        return;

    QByteArray      data = _client->readAll();
    QJsonParseError error;
    QJsonDocument   doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError)
        return;

    QJsonObject json = doc.object();
    if (json.contains("username") && _clients[_client].isEmpty())
    {
        handleAuthRequest(_client, json);
    }
    else if (json.contains("request") && !_clients[_client].isEmpty())
    {
        handleResourceRequest(_client, json);
    }
}

void ResourcesServer::onDisconnected()
{
    _client = qobject_cast<QTcpSocket*>(sender());
    if (!_client)
        return;
    if (_clients.contains(_client) && !_clients[_client].isEmpty())
    {
        qInfo() << "Отключение клиента" << _clients[_client];
    }
    _clients.remove(_client);
    _client->deleteLater();
    emit usersUpdated();
}

void ResourcesServer::onNewConnection()
{
    if (_clients.size() >= _clientLimit)
    {
        _client = _tcpServer->nextPendingConnection();
        _client->disconnectFromHost();
        return;
    }

    _client = _tcpServer->nextPendingConnection();
    if (_bannedUsers.contains(_client->peerAddress().toString()))
    {
        qInfo() << "Попытка присоединится с заблокированнго IP" << _client->peerAddress().toString();
        _client->disconnectFromHost();
        return;
    }

    _clients[_client] = "";

    connect(_client, &QTcpSocket::readyRead, this, &ResourcesServer::onReadyRead);
    connect(_client, &QTcpSocket::disconnected, this, &ResourcesServer::onDisconnected);
}

bool ResourcesServer::isPermittedUser(const QString& username)
{
    return _permittedUsers.contains(username);
}

void ResourcesServer::handleAuthRequest(QTcpSocket* client, const QJsonObject& json)
{

    if (_rejectAuth)
    {
        _clients.remove(client);
        client->disconnectFromHost();
        return;
    }

    QString username = json["username"].toString();
    if (_clients.values().contains(username))
    {
        _clients.remove(client);
        client->disconnectFromHost();
        return;
    }
    if (isPermittedUser(username))
    {
        _clients[client] = username;
        qInfo() << "Клиент" << username << client->peerAddress().toString() << "авторизован";
        emit usersUpdated();
    }
    else
    {
        qInfo() << "Имя" << username << "не в списке разрешённых";
        qInfo() << "IP" << client->peerAddress().toString() << "заблокирован";
        _bannedUsers.append(client->peerAddress().toString());
        _clients.remove(client);
        client->disconnectFromHost();
    }
}

void ResourcesServer::handleResourceRequest(QTcpSocket* client, const QJsonObject& json)
{
    if (_rejectRc)
    {
        return;
    }

    QString username = json["username"].toString();
    int     time     = static_cast<int>(json["time"].toDouble());
    int     request  = static_cast<uint32_t>(json["request"].toDouble());

    uint8_t arr[4] = { 0 };

    memcpy(arr, &request, sizeof(request));

    for (int i = 0; i < 4; i++)
    {
        if (arr[i] != 0)
        {
            if (!_resourcesTimers[i]->isActive())
            {
                QTcpSocket* tmp = _clients.key(_resources[i], nullptr);
                if (tmp && tmp->isValid())
                    sendResponse(_clients.key(_resources[i]), _resources[i], i + 1, 2);

                _resources[i] = username;
                _resourcesTimers[i]->setSingleShot(true);
                _resourcesTimers[i]->start(_zTime);

                _time[username] = time;
                if (!_resourceTimes.contains(username))
                    _resourceTimes[username] = QTime::currentTime();

                sendResponse(client, username, i + 1, 1);

                qInfo() << "Клиенту" << username << "выделен ресурс" << i + 1;
                emit resourcesUpdated();
            }
            else
            {
                sendResponse(client, username, i + 1, 0);
                qInfo() << "Клиенту" << username << "отклонен запрос на ресурс" << i + 1;
            }
        }
    }
    for (auto& un : _permittedUsers)
    {
        if (!_resources.values().contains(un))
        {
            _resourceTimes.remove(un);
        }
    }
}

void ResourcesServer::sendResponse(QTcpSocket* client, const QString& username, int resource, int status)
{
    QJsonObject response;
    response["username"] = username;
    response["resource"] = resource;
    response["status"]   = status;

    QJsonDocument doc(response);
    client->write(doc.toJson());
}

void ResourcesServer::loadSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);

    settings.beginGroup("Server");
    setPort(settings.value("Port", 90).toInt());
    setClientLimit(settings.value("ClientLimit", 20).toInt());
    setZTime(settings.value("RequestTime", 30000).toInt());
    settings.endGroup();

    settings.beginGroup("PermittedUsers");
    _permittedUsers = settings.value("Users").toStringList();
    settings.endGroup();
}

QMap<QString, QTime> ResourcesServer::resourceTimes() const
{
    return _resourceTimes;
}

QMap<int, QString> ResourcesServer::resources() const
{
    return _resources;
}

const QMap<QTcpSocket*, QString>& ResourcesServer::clients() const
{
    return _clients;
}
