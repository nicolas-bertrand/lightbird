#include <QtPlugin>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include "IMime.h"
#include "ITableFiles.h"

#include "Execute.h"
#include "Medias.h"
#include "Plugin.h"
#include "Uploads.h"

Plugin  *Plugin::instance = NULL;

Plugin &Plugin::getInstance()
{
    return (*Plugin::instance);
}

Plugin::Plugin()
{

}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    Plugin::instance = this;
    this->daysOfWeek[1] = "Mon";
    this->daysOfWeek[2] = "Tue";
    this->daysOfWeek[3] = "Wed";
    this->daysOfWeek[4] = "Thu";
    this->daysOfWeek[5] = "Fri";
    this->daysOfWeek[6] = "Sat";
    this->daysOfWeek[7] = "Sun";
    this->months[1] = "Jan";
    this->months[2] = "Feb";
    this->months[3] = "Mar";
    this->months[4] = "Apr";
    this->months[5] = "May";
    this->months[6] = "Jun";
    this->months[7] = "Jul";
    this->months[8] = "Aug";
    this->months[9] = "Sep";
    this->months[10] = "Oct";
    this->months[11] = "Nov";
    this->months[12] = "Dec";
    this->interfaces.push_back("desktop");
    this->interfaces.push_back("web");
    this->interfaces.push_back("mobile");
    Medias::getInstance(this);
    Uploads::getInstance(this);
    return (true);
}

void    Plugin::onUnload()
{
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    this->api = api;
}

void    Plugin::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "Http Client";
    metadata.brief = "The LightBird Http Client.";
    metadata.description = "This client is design to offer a simple access to the server features via a web browser.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

void    Plugin::onUnserialize(LightBird::IClient &client, LightBird::IOnUnserialize::Unserialize type)
{
    LightBird::IRequest &request = client.getRequest();

    if (!request.isError() && (request.getMethod() == "POST" || request.getMethod() == "PUT"))
    {
        if (type == LightBird::IOnUnserialize::IDoUnserializeHeader && request.getUri().path().contains("/StartUpload"))
            Uploads::getInstance().start(client);
        else if (type == LightBird::IOnUnserialize::IDoUnserializeContent)
            Uploads::getInstance().progress(client);
    }
}

bool        Plugin::doExecution(LightBird::IClient &client)
{
    QString uri;
    QString interface;
    QString path;

    // Set the generic HTTP headers
    client.getResponse().getHeader().insert("server", "LightBird");
    client.getResponse().getHeader().insert("date", this->httpDate(QDateTime::currentDateTime().toUTC()));
    client.getResponse().getHeader().insert("cache-control", "private");
    // Create the session cookie
    this->_cookie(client);
    // Get the uri of the request
    uri = client.getRequest().getUri().toString(QUrl::RemoveScheme | QUrl::RemoveAuthority |
                                                QUrl::RemoveQuery | QUrl::RemoveFragment);
    uri = QDir::cleanPath(uri);
    if (uri.at(0) == '/')
        uri = uri.right(uri.size() - 1);
    // Remove "Client" of the uri
    uri.remove(QRegExp("^Client/"));
    // Try to identify the user
    this->_identify(client, uri);
    // Find the interface of the user
    interface = this->_getInterface(client);
    // The blank uri does nothing
    if (uri == "blank")
        return (true);
    // If the characters ".." are in the uri, it is unvalid
    if (uri.contains("..") || uri.contains("~"))
        this->response(client, 403, "Forbidden", "The uri in the header must not contains \"..\" or \"~\".");
    // The client wants to execute something
    else if (uri.left(8) == "Execute/")
        Execute(*this->api, client, uri.right(uri.size() - uri.indexOf('/') - 1));
    // The client wants to download a file
    else
    {
        if (uri.isEmpty())
            uri = "index.html";
        path = this->api->getPluginPath() + "/www/" + interface + "/" + uri;
        // If there an id in the uri, the file is in the filesPath
        if (!client.getRequest().getUri().queryItemValue("id").isEmpty())
            this->_getFile(client);
        // If the file exists in the www directory
        else if (QFileInfo(path).isFile())
        {
            // Set the MIME type of the file from the IMime extension
            client.getResponse().setType(this->_getMime(uri));
            client.getResponse().getContent().setStorage(LightBird::IContent::FILE, path);
        }
        // If the help has not been found, an error occured
        else if (uri.contains("/"))
            this->response(client, 404, "Not Found", "File not found.");
        // Otherwise the client is redirected
        else
        {
            this->response(client, 303, "See Other", "File not found.");
            client.getResponse().getHeader().insert("location", "/");
        }
    }
    return (true);
}

bool        Plugin::onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type)
{
    if (type == LightBird::IOnSerialize::IDoSerializeContent && !client.getRequest().isError())
        Medias::getInstance().update(client);
    return (true);
}

void        Plugin::onFinish(LightBird::IClient &client)
{
    Medias::getInstance().onFinish(client);
    // Check if the session of the client has to be destroyed
    if (!this->destroySessions.contains(client.getId()))
        return ;
    QListIterator<unsigned short>   portsIt(this->api->network().getPorts());
    QStringList                     protocols;
    unsigned int                    maxClients;
    QStringList                     clients;
    LightBird::INetwork::Client     clientInfo;
    QString                         sid;

    // Disconnect all the clients of the session to destroy
    this->destroySessions.removeAll(client.getId());
    sid = client.getInformations().value("sid").toString();
    // Get the number of the HTTP ports
    while (portsIt.hasNext())
    {
        this->api->network().getPort(portsIt.peekNext(), protocols, maxClients);
        if (protocols.contains("http", Qt::CaseInsensitive))
            clients.append(this->api->network().getClients(portsIt.peekNext()));
        portsIt.next();
    }
    // Disconnect all the connections of the account to disconnect, on the port of the web client
    QStringListIterator it(clients);
    while (it.hasNext())
    {
        this->api->network().getClient(it.peekNext(), clientInfo);
        if (clientInfo.informations.value("sid") == sid)
            this->api->network().disconnect(it.peekNext());
        it.next();
    }
}

void    Plugin::onDisconnect(LightBird::IClient &client)
{
    Medias::getInstance().disconnected(client);
    Uploads::getInstance().disconnected(client);
}

bool    Plugin::timer(const QString &name)
{
    QDateTime connected = QDateTime::currentDateTime().addSecs(-(60 * 60));
    QDateTime disconnected = QDateTime::currentDateTime().addSecs(-60);

    // Remove expired sessions
    if (name == "session")
    {
        this->sessionsMutex.lock();
        QMutableMapIterator<QString, Session> it(this->sessions);
        while (it.hasNext())
        {
            it.next();
            // If the user is connected
            if (!it.value().getAccount().getId().isEmpty())
            {
                // The session has not been updated for too long
                if (it.value().getUpdate() < connected)
                    it.remove();
            }
            // The session is disconnected for too long
            else if (it.value().getCreation() < disconnected)
                it.remove();
        }
        this->sessionsMutex.unlock();
    }
    return (true);
}

LightBird::IApi &Plugin::getApi()
{
    return (*this->api);
}

void    Plugin::response(LightBird::IClient &client, int code, const QString &message, const QByteArray &content)
{
    client.getResponse().setCode(code);
    client.getResponse().setMessage(message);
    if (!content.isEmpty())
        client.getResponse().getContent().setContent(content, false);
}

QString Plugin::httpDate(const QDateTime &date, bool separator)
{
    QString s = " ";

    if (separator)
        s = "-";
    return (this->daysOfWeek[date.date().dayOfWeek()] + ", " + date.toString("dd") +
            s + this->months[date.date().month()] + s + date.toString("yyyy") + " " +
            date.toString("hh:mm:ss")+ " GMT");
}

void    Plugin::removeSession(LightBird::IClient &client)
{
    this->sessionsMutex.lock();
    this->destroySessions.push_back(client.getId());
    this->sessions.remove(client.getInformations().value("sid").toString());
    this->sessionsMutex.unlock();
}

void        Plugin::_cookie(LightBird::IClient &client)
{
    QString sid = this->_getCookie(client, "sid");

    this->sessionsMutex.lock();
    // If the session id doesn't exists or it is unknow, the cookie is created
    if (sid.isEmpty() || !this->sessions.contains(sid))
        this->_createCookie(client);
    // Otherwise, the session id is saved and updated
    else
    {
        client.getInformations().insert("sid", sid);
        this->sessions[sid].setUpdate();
    }
    this->sessionsMutex.unlock();
}

void    Plugin::_createCookie(LightBird::IClient &client)
{
    Session session;
    QString cookie;

    // Add the Set-Cookie HTTP header
    cookie = "sid=" + session.getId() + "; expires=";
    cookie += this->httpDate(QDateTime::currentDateTime().addYears(2), true);
    cookie += "; path=/";
    client.getResponse().getHeader().insertMulti("set-cookie", cookie);
    // Save the session id
    client.getInformations().insert("sid", session.getId());
    this->sessions.insert(session.getId(), session);
}

void    Plugin::_identify(LightBird::IClient &client, const QString &uri)
{
    QString identifiant = this->_getCookie(client, "identifiant");
    QString sid;
    QString aid;

    // If there no identifiant, the user is not connected
    if (identifiant.isEmpty())
        return (client.getAccount().clear());
    sid = client.getInformations().value("sid").toString();
    // Try to identiy the user
    if (client.getAccount().setIdFromIdentifiantAndSalt(identifiant, sid))
    {
        this->sessionsMutex.lock();
        // Connect the session
        if ((aid = this->sessions[sid].getAccount().getId()).isEmpty())
            this->sessions[sid].getAccount().setId(client.getAccount().getId());
        // Check that the session account is the same
        else if (aid != client.getAccount().getId())
            // Disconnect the client
            client.getAccount().clear();
        this->sessionsMutex.unlock();
    }
    // If the client try to identify, an error occured
    else if (uri == "blank")
    {
        this->response(client, 403, "Forbidden", "Access denied.");
        // Delete the cookie if it is not correct
        client.getResponse().getHeader().insertMulti("set-cookie", "identifiant=; path=/");
    }
}

QString Plugin::_getInterface(LightBird::IClient &client)
{
    QString interface;

    // Get the name of the interface from the cookies
    if (!(interface = this->_getCookie(client, "interface")).isEmpty())
        // The interface doesn't exists
        if (!this->interfaces.contains(interface))
            interface.clear();
    // Save the interface in the client informations
    if (!interface.isEmpty())
        client.getInformations().insert("interface", interface);
    // Get the interface from the client informations
    else if ((interface = client.getInformations().value("interface").toString()).isEmpty())
        interface = DEFAULT_INTERFACE_NAME;
    return (interface);
}

void    Plugin::_getFile(LightBird::IClient &client)
{
    QSharedPointer<LightBird::ITableFiles> file(this->api->database().getFiles(client.getRequest().getUri().queryItemValue("id")));
    QString path;

    // Check that the user can access to the file
    if (!file->isAllowed(client.getAccount().getId(), "read"))
        return (this->response(client, 403, "Forbidden", "Access denied."));
    path = file->getFullPath();
    // Check that the file extsis on the disk
    if (!QFileInfo(path).isFile())
        return (this->response(client, 404, "Not Found", "File not found."));
    // If the file has to be downloaded by the browser, a special MIME is used
    if (client.getRequest().getUri().queryItemValue("download") != "true")
        client.getResponse().setType(this->_getMime(path));
    else
        client.getResponse().setType(DEFAULT_CONTENT_TYPE);
    client.getResponse().getContent().setStorage(LightBird::IContent::FILE, path);
}

QString             Plugin::_getMime(const QString &file)
{
    QList<void *>   extensions;
    QString         result;

    if (!file.contains('.'))
        return (result);
    extensions = this->api->extensions().get("IMime");
    if (!extensions.isEmpty())
        result = static_cast<LightBird::IMime *>(extensions.first())->getMime(file);
    this->api->extensions().release(extensions);
    return (result);
}

QString     Plugin::_getCookie(LightBird::IClient &client, const QString &n)
{
    QString name(n + "=");
    QString cookies(client.getRequest().getHeader().value("cookie"));

    if (!cookies.contains(name))
        return ("");
    QStringListIterator it(cookies.split(";"));
    while (it.hasNext())
    {
        if (it.peekNext().trimmed().left(name.size()) == name)
            return (it.peekNext().right(it.peekNext().size() - it.peekNext().indexOf('=') - 1));
        it.next();
    }
    return ("");
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
