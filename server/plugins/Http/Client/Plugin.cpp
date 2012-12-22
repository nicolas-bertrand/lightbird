#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QUrlQuery>

#include "IMime.h"

#include "LightBird.h"
#include "Medias.h"
#include "Plugin.h"

Plugin  *Plugin::_instance = NULL;

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->_api = api;
    Plugin::_instance = this;
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
    if ((this->wwwDir = this->_api->configuration(true).get("resources/resource")).isEmpty())
        this->wwwDir = "www";
    // This timer will remove the outdated data in this->attempts
    this->_api->timers().setTimer("attempts", IDENTIFICATION_TIME * 1000);
    return (true);
}

void    Plugin::onUnload()
{
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    this->_api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    this->_api = api;
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

void        Plugin::onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type)
{
    QString uri;
    LightBird::IRequest &request = client.getRequest();

    // Checks the header and identify the client
    if (type == LightBird::IOnDeserialize::IDoDeserializeHeader)
    {
        // Sets the generic HTTP headers
        client.getResponse().getHeader().insert("server", "LightBird");
        client.getResponse().getHeader().insert("date", this->httpDate(QDateTime::currentDateTimeUtc()));
        client.getResponse().getHeader().insert("cache-control", "no-cache");
        // Gets the uri of the request
        uri = client.getRequest().getUri().toString(QUrl::RemoveScheme | QUrl::RemoveAuthority | QUrl::RemoveQuery | QUrl::RemoveFragment);
        uri = LightBird::cleanPath(uri.remove(".."), true).remove(QRegExp("^Client/"));
        client.getRequest().getInformations().insert("uri", uri);
        // Manages the session
        this->_session(client);
    }
    // If an error occured we don't execute the request
    else if (type == LightBird::IOnDeserialize::IDoDeserialize && client.getResponse().getCode() >= 400)
        request.setError(true);
    // The client is uploading something
    if (request.getHeader().contains("content-length"))
    {
        // One must be identified to upload data
        if (!client.getAccount().exists())
            this->_api->network().disconnect(client.getId());
        // Manages the upload of files
        else if (!request.isError() && !client.getResponse().isError() && request.getUri().path().endsWith("/command/uploads"))
        {
            if (type == LightBird::IOnDeserialize::IDoDeserializeHeader)
                this->_uploads.onDeserializeHeader(client);
            else if (type == LightBird::IOnDeserialize::IDoDeserializeContent)
                this->_uploads.onDeserializeContent(client);
        }
        else if (!request.getUri().path().contains("/commands/uploads"))
            this->_api->network().disconnect(client.getId());
    }
}

bool        Plugin::doExecution(LightBird::IClient &client)
{
    QString uri = client.getRequest().getInformations().value("uri").toString();
    QString interface;
    QString path;

    // The blank uri does nothing
    if (uri == "blank")
        return (true);
    // Finds the interface of the user
    interface = this->_getInterface(client);
    // The client wants to execute a command
    if (uri.startsWith("command/"))
        this->commands.execute(client, uri.right(uri.size() - uri.indexOf('/') - 1));
    else if (uri == "Translation.js")
        this->_translation(client, interface);
    // The client wants to download a file
    else
    {
        if (uri.isEmpty())
            uri = "index.html";
        path = this->_api->getPluginPath() + this->wwwDir + "/" + interface + "/" + uri;
        // If there a file id in the uri, the file is in the filesPath
        if (!QUrlQuery(client.getRequest().getUri()).queryItemValue("fileId").isEmpty())
            this->_getFile(client);
        // If the file exists in the www directory
        else if (QFileInfo(path).isFile())
        {
            // Set the MIME type of the file from the IMime extension
            client.getResponse().setType(this->_getMime(uri));
            client.getResponse().getContent().setStorage(LightBird::IContent::FILE, path);
        }
        // If the folder has not been found, an error occured
        else if (uri.contains("/") || uri.contains("favicon"))
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
        this->_medias.update(client);
    return (true);
}

void        Plugin::onFinish(LightBird::IClient &client)
{
    this->_medias.onFinish(client);
    this->_uploads.onFinish(client);
}

bool    Plugin::onDisconnect(LightBird::IClient &client)
{
    return (!client.getInformations().contains("delay_disconnection"));
}

void    Plugin::onDestroy(LightBird::IClient &client)
{
    this->_medias.disconnected(client);
    this->_uploads.onDestroy(client);
}

bool    Plugin::timer(const QString &name)
{
    // Removes the outdated data in this->attempts
    if (name == "attempts")
    {
        this->mutex.lock();
        QMutableHashIterator<QHostAddress, QPair<QDateTime, quint32> > it(this->attempts);
        while (it.hasNext())
        {
            it.next();
            if (it.value().first < QDateTime::currentDateTime())
                it.remove();
        }
        this->mutex.unlock();
    }
    return (true);
}

void    Plugin::_session(LightBird::IClient &client)
{
    QString id;
    QString sessionId = QUrlQuery(client.getRequest().getUri()).queryItemValue("sessionId");
    QString identifiant = QUrlQuery(client.getRequest().getUri()).queryItemValue("identifiant");
    LightBird::Session session;

    // If the session id or the identifiant doesn't exists, the account is cleared
    if (sessionId.isEmpty() || (session = this->_api->sessions().getSession(sessionId)).isNull() ||
        (!identifiant.isEmpty() && !this->_checkIdentifiant(client, session, identifiant)))
    {
        // Ensures that the client is not associated with a session
        if (!(id = client.getAccount().getId()).isEmpty() && !client.getSessions(id).isEmpty())
        {
            QStringListIterator it(client.getSessions(id));
            while (it.hasNext())
                this->_api->sessions().getSession(it.next())->removeClient(client.getId());
        }
        client.getAccount().clear();
    }
    // Otherwise the client is identified
    else if (!identifiant.isEmpty())
    {
        // Ensures that it is associated to the session
        session->setClient(client.getId());
        if (client.getAccount().getId() != session->getAccount())
            client.getAccount().setId(session->getAccount());
        // Update the expiration date of the session
        if (session->getExpiration() < QDateTime::currentDateTime().addDays(7))
            session->setExpiration(QDateTime::currentDateTime().addMonths(1));
    }
}

bool    Plugin::_checkIdentifiant(LightBird::IClient &client, LightBird::Session &session, const QString &identifiant)
{
    if (identifiant == session->getInformation("identifiant"))
        return (true);
    this->identificationFailed(client);
    client.getResponse().setError(true);
    // Tells the client that it is not identified
    if (client.getRequest().getInformations().value("uri").toString() == "blank")
        this->response(client, 403, "Forbidden");
    return (false);
}

QString     Plugin::_getInterface(LightBird::IClient &client)
{
    QString interface;

    // Gets the name of the interface from the cookies
    if (!(interface = this->getCookie(client, "interface")).isEmpty())
        // The interface doesn't exists
        if (!this->interfaces.contains(interface))
            interface.clear();
    // Saves the interface in the client informations
    if (!interface.isEmpty())
        client.getInformations().insert("interface", interface);
    // Gets the interface from the client informations
    else if ((interface = client.getInformations().value("interface").toString()).isEmpty())
        interface = DEFAULT_INTERFACE_NAME;
    return (interface);
}

void    Plugin::_translation(LightBird::IClient &client, const QString &interface)
{
    QString path = this->_api->getPluginPath() + this->wwwDir + "/" + interface + "/languages/";
    QString language = this->getCookie(client, "language");

    // Tries to get the translation in the language asked by the client in the cookie
    if (language.size() == 2 && QFileInfo(path + language + ".js").isFile())
        path += language + ".js";
    // Otherwise if there is a translation in the language of the server we use it
    else if (QFileInfo(path + this->_api->getLanguage() + ".js").isFile())
        path += this->_api->getLanguage() + ".js";
    // English is the default language
    else
        path += "en.js";
    // Sends the translation
    client.getResponse().setType(this->_getMime(path));
    client.getResponse().getContent().setStorage(LightBird::IContent::FILE, path);
}

void    Plugin::_getFile(LightBird::IClient &client)
{
    LightBird::TableFiles   file(QUrlQuery(client.getRequest().getUri()).queryItemValue("fileId"));
    QString path;

    // Checks that the user can access to the file
    if (client.getAccount().getId().isEmpty() || !file.isAllowed(client.getAccount().getId(), "read"))
        return (this->response(client, 403, "Forbidden", "Access denied."));
    path = file.getFullPath();
    // Checks that the file exists on the file system
    if (!QFileInfo(path).isFile())
        return (this->response(client, 404, "Not Found", "File not found."));
    // If the file has to be downloaded by the browser, a special MIME is used
    if (QUrlQuery(client.getRequest().getUri()).queryItemValue("download") != "true")
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
    extensions = this->_api->extensions().get("IMime");
    if (!extensions.isEmpty())
        result = static_cast<LightBird::IMime *>(extensions.first())->getMime(file);
    this->_api->extensions().release(extensions);
    return (result);
}

void    Plugin::response(LightBird::IClient &client, int code, const QString &message, const QByteArray &content)
{
    client.getResponse().setCode(code);
    client.getResponse().setMessage(message);
    if (!content.isEmpty())
        client.getResponse().getContent().setData(content, false);
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
        cookie += "expires=" + Plugin::instance().httpDate(QDateTime::currentDateTimeUtc().addYears(2), true) + "; ";
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

bool        Plugin::identificationAllowed(LightBird::IClient &client)
{
    bool    result = true;

    this->mutex.lock();
    if (this->attempts.value(client.getPeerAddress()).second >= IDENTIFICATION_ATTEMPTS &&
        this->attempts.value(client.getPeerAddress()).first > QDateTime::currentDateTime())
        result = false;
    this->mutex.unlock();
    return (result);
}

void    Plugin::identificationFailed(LightBird::IClient &client)
{
    this->mutex.lock();
    this->attempts[client.getPeerAddress()].first = QDateTime::currentDateTime().addSecs(IDENTIFICATION_TIME);
    this->attempts[client.getPeerAddress()].second++;
    this->mutex.unlock();
}

Plugin  &Plugin::instance()
{
    return (*Plugin::_instance);
}

LightBird::IApi &Plugin::api()
{
    return (*Plugin::_instance->_api);
}

Files   &Plugin::files()
{
    return (Plugin::_instance->_files);
}

Medias  &Plugin::medias()
{
    return (Plugin::_instance->_medias);
}

Uploads &Plugin::uploads()
{
    return (Plugin::_instance->_uploads);
}
