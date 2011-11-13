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
    if ((this->wwwDir = this->api->configuration(true).get("resources/resource")).isEmpty())
        this->wwwDir = "www";
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
    // Manage the session cookie
    this->_session(client);
    // Get the uri of the request
    uri = client.getRequest().getUri().toString(QUrl::RemoveScheme | QUrl::RemoveAuthority |
                                                QUrl::RemoveQuery | QUrl::RemoveFragment);
    uri = QDir::cleanPath(uri);
    if (uri.at(0) == '/')
        uri = uri.right(uri.size() - 1);
    // Remove "Client" of the uri
    uri.remove(QRegExp("^Client/"));
    // Find the interface of the user
    interface = this->_getInterface(client);
    // The blank uri does nothing
    if (uri == "blank")
        return (true);
    // The characters ".." and "~" are forbidden in the uri
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
        path = this->api->getPluginPath() + this->wwwDir + "/" + interface + "/" + uri;
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
    // The session is destroyed and all the clients associated to it are disconnected
    if (client.getInformations().contains("disconnect") && !client.getSessions().isEmpty())
        this->api->sessions().destroy(client.getSessions().first(), true);
}

void    Plugin::onDisconnect(LightBird::IClient &client)
{
    Medias::getInstance().disconnected(client);
    Uploads::getInstance().disconnected(client);
}

void        Plugin::_session(LightBird::IClient &client)
{
    QString sid = this->getCookie(client, "sid");
    QString id = this->getCookie(client, "identifiant");
    LightBird::Session session;

    // If the session or the identifiant doesn't exists, the cookie and the account are cleared
    if (sid.isEmpty() || (session = this->api->sessions().getSession(sid)).isNull() ||
        (!id.isEmpty() && (session->getInformation("identifiant") != id)))
    {
        client.getResponse().getHeader().remove("set-cookie");
        this->addCookie(client, "sid");
        this->addCookie(client, "identifiant");
        // Ensures that the client is not associated with a session
        if (!(id = client.getAccount().getId()).isEmpty() && !client.getSessions(id).isEmpty())
        {
            QStringListIterator it(client.getSessions(id));
            while (it.hasNext())
                this->api->sessions().getSession(it.next())->removeClient(client.getId());
        }
        client.getAccount().clear();
        if (!id.isEmpty())
            this->response(client, 403, "Forbidden");
    }
    // Otherwise the client is identified
    else if (!id.isEmpty())
    {
        // Ensure that it is associated with the session
        this->api->sessions().getSession(sid)->setClient(client.getId());
        if (client.getAccount().getId() != session->getAccount())
            client.getAccount().setId(session->getAccount());
        // Update the expiration date of the session
        if (session->getExpiration() < QDateTime::currentDateTime().addDays(7))
            session->setExpiration(QDateTime::currentDateTime().addMonths(1));
    }
}

QString     Plugin::_getInterface(LightBird::IClient &client)
{
    QString interface;

    // Get the name of the interface from the cookies
    if (!(interface = this->getCookie(client, "interface")).isEmpty())
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

QString     Plugin::getCookie(LightBird::IClient &client, const QString &n)
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

void        Plugin::addCookie(LightBird::IClient &client, const QString &name, const QString &value)
{
    QString cookie;

    cookie = name + "=" + value + "; ";
    if (!value.isEmpty())
        cookie += "expires=" + Plugin::getInstance().httpDate(QDateTime::currentDateTime().addYears(2), true) + "; ";
    cookie += "path=/";
    client.getResponse().getHeader().insertMulti("set-cookie", cookie);
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

Q_EXPORT_PLUGIN2(plugin, Plugin)
