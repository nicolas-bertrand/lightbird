#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

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
    _api = api;
    Plugin::_instance = this;
    _daysOfWeek[1] = "Mon";
    _daysOfWeek[2] = "Tue";
    _daysOfWeek[3] = "Wed";
    _daysOfWeek[4] = "Thu";
    _daysOfWeek[5] = "Fri";
    _daysOfWeek[6] = "Sat";
    _daysOfWeek[7] = "Sun";
    _months[1] = "Jan";
    _months[2] = "Feb";
    _months[3] = "Mar";
    _months[4] = "Apr";
    _months[5] = "May";
    _months[6] = "Jun";
    _months[7] = "Jul";
    _months[8] = "Aug";
    _months[9] = "Sep";
    _months[10] = "Oct";
    _months[11] = "Nov";
    _months[12] = "Dec";
    _interfaces.push_back("desktop");
    _interfaces.push_back("web");
    _interfaces.push_back("mobile");
    if ((_wwwDir = _api->configuration(true).get("resources/resource")).isEmpty())
        _wwwDir = "www";

    // This timer will remove the outdated data in attempts
    _api->timers().setTimer("attempts", IDENTIFICATION_TIME * 1000);
    _api->contexts().declareInstance("client", &_context);

    // Creates the files extensions types json
    QFile file(LightBird::c().filesExtensionsPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        _api->log().warning("Unable to open the files extensions json", Properties("file", LightBird::c().filesExtensionsPath).toMap(), "Plugin", "onLoad");
        return (true);
    }
    QJsonObject object = QJsonDocument::fromJson(file.readAll()).object();
    QJsonObject types;
    for (QJsonObject::iterator it(object.begin()); it != object.end(); ++it)
        types.insert(it.key(), it.value().toObject().value("type"));
    _filesExtensionsTypesJson = QJsonDocument(types).toJson(QJsonDocument::Compact);
    return (true);
}

void    Plugin::onUnload()
{
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    _api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    _api = api;
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

void    Plugin::onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type)
{
    if (type != LightBird::IOnDeserialize::IDoDeserializeHeader)
        return ;
    // Sets the name of the context based on the URL
    QString url = client.getRequest().getUri().toString(QUrl::RemoveScheme | QUrl::RemoveAuthority | QUrl::RemoveQuery | QUrl::RemoveFragment);
    if (url.startsWith('/'))
        url.remove(0, 1);
    if (url.isEmpty() || url.startsWith("c/"))
    {
        client.getContexts() << "client";
        // Calls onDeserialize since the next context has not been taken into account yet
        onDeserializeContext(client, type);
    }
}

void    Plugin::onDeserializeContext(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type)
{
    QString uri;
    LightBird::IRequest &request = client.getRequest();

    // Checks the header and identify the client
    if (type == LightBird::IOnDeserialize::IDoDeserializeHeader)
    {
        // Sets the generic HTTP headers
        client.getResponse().getHeader().insert("server", "LightBird");
        client.getResponse().getHeader().insert("date", httpDate(QDateTime::currentDateTimeUtc()));
        client.getResponse().getHeader().insert("cache-control", "no-cache");
        // Gets the uri of the request
        uri = client.getRequest().getUri().toString(QUrl::RemoveScheme | QUrl::RemoveAuthority | QUrl::RemoveQuery | QUrl::RemoveFragment);
        uri = LightBird::cleanPath(uri.remove(".."), true).remove(QRegExp("^c/"));
        client.getRequest().getInformations().insert("uri", uri);
        // Manages the session
        _session(client, uri);
    }
    // If an error occurred we don't execute the request
    else if (type == LightBird::IOnDeserialize::IDoDeserialize && client.getResponse().getCode() >= 400)
        request.setError(true);
    // The client is uploading something
    if (request.getHeader().contains("content-length"))
    {
        uri = request.getUri().path();
        // One must be identified to upload data
        if (!client.getAccount().exists())
            _api->network().disconnect(client.getId(), true);
        // Manages the upload of files
        else if (uri.contains("/command/uploads"))
        {
            if (request.isError() || client.getResponse().isError())
                return ;
            _uploads.onDeserialize(client, type, uri);
        }
        else if (uri.endsWith("/command/files/delete"))
        {
            if (type == LightBird::IOnDeserialize::IDoDeserializeContent)
                _files.deleteFiles(client);
        }
        // The url is not authorized to upload something
        else
            _api->network().disconnect(client.getId(), true);
    }
}

bool    Plugin::doExecution(LightBird::IClient &client)
{
    QString uri = client.getRequest().getInformations().value("uri").toString();
    QString interface;
    QString path;

    // The blank uri does nothing
    if (uri == "blank")
        return (true);
    // Finds the interface of the user
    interface = _getInterface(client);
    // The client wants to execute a command
    if (uri.startsWith("command/"))
        _commands.execute(client, uri.right(uri.size() - uri.indexOf('/') - 1));
    else if (uri == "Translation.js")
        _translation(client, interface);
    else if (uri == "filesExtensionsTypes.json")
        _filesExtensionsTypes(client);
    // The client wants to download a file
    else
    {
        if (uri.isEmpty())
            uri = "index.html";
        path = _api->getPluginPath() + _wwwDir + "/" + interface + "/" + uri;
        // If there a file id in the uri, the file is in the filesPath
        if (!QUrlQuery(client.getRequest().getUri()).queryItemValue("fileId").isEmpty())
            _getFile(client);
        // If the file exists in the www directory
        else if (QFileInfo(path).isFile())
        {
            // Sets the MIME type of the file
            client.getResponse().setType(LightBird::getFileMime(uri));
            client.getResponse().getContent().setStorage(LightBird::IContent::FILE, path);
        }
        // If the folder has not been found, an error occurred
        else if (uri.contains("/") || uri.contains("favicon"))
            response(client, 404, "Not Found", "File not found.");
        // Otherwise the client is redirected
        else
        {
            response(client, 303, "See Other", "File not found.");
            client.getResponse().getHeader().insert("location", "/");
        }
    }
    return (true);
}

bool    Plugin::onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type)
{
    if (type == LightBird::IOnSerialize::IDoSerializeContent && !client.getRequest().isError())
        _medias.update(client);
    return (true);
}

void    Plugin::onFinish(LightBird::IClient &client)
{
    _medias.onFinish(client);
    _uploads.onFinish(client);
    client.getContexts().removeAll("client");
}

bool    Plugin::onDisconnect(LightBird::IClient &client)
{
    return (!client.getInformations().contains("delay_disconnection"));
}

void    Plugin::onDestroy(LightBird::IClient &client)
{
    _medias.disconnected(client);
    _uploads.onDestroy(client);
}

bool    Plugin::timer(const QString &name)
{
    // Removes the outdated data in _attempts
    if (name == "attempts")
    {
        _mutex.lock();
        QMutableHashIterator<QHostAddress, QPair<QDateTime, quint32> > it(_attempts);
        while (it.hasNext())
        {
            it.next();
            if (it.value().first < QDateTime::currentDateTime())
                it.remove();
        }
        _mutex.unlock();
    }
    return (true);
}

void    Plugin::_session(LightBird::IClient &client, const QString &uri)
{
    QString id;
    QString sessionId = QUrlQuery(client.getRequest().getUri()).queryItemValue("sessionId");
    QString identifiant = QUrlQuery(client.getRequest().getUri()).queryItemValue("identifiant");
    LightBird::Session session;

    // If the session id or the identifiant does not exist, the account is cleared
    if (sessionId.isEmpty() || (session = _api->sessions().getSession(sessionId)).isNull() ||
        (!identifiant.isEmpty() && !_checkIdentifiant(client, session, identifiant)))
    {
        // Ensures that the client is not associated with a session
        if (!(id = client.getAccount().getId()).isEmpty() && !client.getSessions(id).isEmpty())
        {
            QStringListIterator it(client.getSessions(id));
            while (it.hasNext())
                _api->sessions().getSession(it.next())->removeClient(client.getId());
        }
        client.getAccount().clear();
        // Tells the client that it is not identified
        if (!identifiant.isEmpty() && uri == "blank")
            response(client, 403, "Forbidden");
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
    identificationFailed(client);
    client.getResponse().setError(true);
    return (false);
}

QString Plugin::_getInterface(LightBird::IClient &client)
{
    QString interface;

    // Gets the name of the interface from the cookies
    if (!(interface = getCookie(client, "interface")).isEmpty())
        // The interface does not exist
        if (!_interfaces.contains(interface))
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
    QString path = _api->getPluginPath() + _wwwDir + "/" + interface + "/languages/";
    QString language = getCookie(client, "language");

    // Tries to get the translation in the language asked by the client in the cookie
    if (language.size() == 2 && QFileInfo(path + language + ".js").isFile())
        path += language + ".js";
    // Otherwise if there is a translation in the language of the server we use it
    else if (QFileInfo(path + _api->getLanguage() + ".js").isFile())
        path += _api->getLanguage() + ".js";
    // English is the default language
    else
        path += "en.js";
    // Sends the translation
    client.getResponse().setType(LightBird::getFileMime(path));
    client.getResponse().getContent().setStorage(LightBird::IContent::FILE, path);
}

void    Plugin::_filesExtensionsTypes(LightBird::IClient &client)
{
    client.getResponse().getContent().setStorage(LightBird::IContent::BYTEARRAY).setData(_filesExtensionsTypesJson, false);
    client.getResponse().setType("application/json");
}

void    Plugin::_getFile(LightBird::IClient &client)
{
    LightBird::TableFiles   file(QUrlQuery(client.getRequest().getUri()).queryItemValue("fileId"));
    QString path;

    // Checks that the user can access to the file
    if (client.getAccount().getId().isEmpty() || !file.isAllowed(client.getAccount().getId(), "read"))
        return (response(client, 403, "Forbidden", "Access denied."));
    path = file.getFullPath();
    // Checks that the file exists on the file system
    if (!QFileInfo(path).isFile())
        return (response(client, 404, "Not Found", "File not found."));
    // If the file has to be downloaded by the browser, a special MIME is used
    if (QUrlQuery(client.getRequest().getUri()).queryItemValue("download") != "true")
        client.getResponse().setType(LightBird::getFileMime(path));
    else
        client.getResponse().setType(DEFAULT_CONTENT_TYPE);
    client.getResponse().getContent().setStorage(LightBird::IContent::FILE, path);
}

void    Plugin::response(LightBird::IClient &client, int code, const QString &message, const QByteArray &content)
{
    client.getResponse().setCode(code);
    client.getResponse().setMessage(message);
    if (!content.isEmpty())
        client.getResponse().getContent().setData(content, false);
}

QString Plugin::getCookie(LightBird::IClient &client, const QString &n)
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

void    Plugin::addCookie(LightBird::IClient &client, const QString &name, const QString &value)
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
    return (_daysOfWeek[date.date().dayOfWeek()] + ", " + date.toString("dd") +
            s + _months[date.date().month()] + s + date.toString("yyyy") + " " +
            date.toString("hh:mm:ss")+ " GMT");
}

bool    Plugin::identificationAllowed(LightBird::IClient &client)
{
    bool    result = true;

    _mutex.lock();
    if (_attempts.value(client.getPeerAddress()).second >= IDENTIFICATION_ATTEMPTS &&
        _attempts.value(client.getPeerAddress()).first > QDateTime::currentDateTime())
        result = false;
    _mutex.unlock();
    return (result);
}

void    Plugin::identificationFailed(LightBird::IClient &client)
{
    _mutex.lock();
    _attempts[client.getPeerAddress()].first = QDateTime::currentDateTime().addSecs(IDENTIFICATION_TIME);
    _attempts[client.getPeerAddress()].second++;
    _mutex.unlock();
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
