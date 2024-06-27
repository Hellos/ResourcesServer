#pragma once

#include <QtPlugin>
/*!
 * \brief Сервер, дающий доступ к ресурсам по запросу пользователей.
 *  Для коммуникации клиент-сервер используется json-формат сообщений.
 *  пример команды на регистрацию: {"username": "test_username"}
 *  пример команды на запрос ресурсов: {"username": "test_username","time": "10000","request": "6243"}
 */
class IResourcesServer
{

public:
    /// @brief Неизменяемые установки
    enum
    {
        ABSOLUTE_CLINET_LIMIT = 20,
        RESOURCE_QUANTITY     = 4
    };

public:
    /// @brief Деструктор
    virtual ~IResourcesServer() = default;

    /// @brief Установить прослушиваемый порт
    virtual void setPort(int) = 0;

    /// @brief Прослушиваемый порт
    virtual int port() const = 0;

    /// @brief Установить предельное число соединений с сервером.
    ///
    /// При досижении предела соединений в ответ на запросы
    /// клиентов о соединении с сервером, последний либо отсылает
    /// уведомление о неудаче запроса, либо ожидает завершения
    /// одного из текущих соединений, после чего соединяется с
    /// новым клиентом
    virtual void setClientLimit(int) = 0;

    /// @brief Установить список разрешенных имен пользователей
    virtual void setPermittedUsers(const QStringList& names) = 0;

    /// @brief Список разрешенных имен пользователей
    virtual QStringList permittedUsers() const = 0;

    /// @brief Установить максимальное время владения клиентом ресурсом
    ///
    /// По истечении этого времени любой клиент может
    /// завладить им без возможности отказа, в том числе и
    /// текущий владелец
    virtual void setZTime(int ms) = 0;

    /// @brief Максимальное время владения клиентом ресурсом
    virtual int zTime() const = 0;

    /// @brief Запуск сервера
    virtual void run() = 0;

    /// @brief Проверка работы сервера
    ///
    /// @sa run()
    virtual bool isRunning() const = 0;

    /// @brief Освободить все занятые ресурсы
    virtual void freeResources() = 0;

    /// @brief Заставляет сервер (не) принимать запросы авторизации
    virtual void rejectAuthRequests(bool reject) = 0;

    /// @brief Заставляет сервер (не) принимать запросы ресурсов
    virtual void rejectRcRequests(bool reject) = 0;

};

Q_DECLARE_INTERFACE(IResourcesServer, "org.stc.test.iresourcesserver");
