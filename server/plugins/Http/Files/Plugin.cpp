#include <QFileInfo>
#include <qmath.h>

#include "IMime.h"
#include "LightBird.h"
#include "Plugin.h"

const QString Plugin::link = "<a href=\"/f/%1\">%2</a>";

Plugin::Plugin()
{
    this->context = new Context(this);
}

Plugin::~Plugin()
{
    delete this->context;
}

bool        Plugin::onLoad(LightBird::IApi *api)
{
    QFile   file;

    this->api = api;
    file.setFileName(this->api->getResourcesPath() + "/directory.html");
    if (!file.open(QIODevice::ReadOnly))
        return (false);
    this->content = file.readAll();
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
    metadata.name = "Http files";
    metadata.brief = "Access to the server files via HTTP.";
    metadata.description = "Allows to display the files and the directories of the server via a simple html interface.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

void    Plugin::getContexts(QMap<QString, QObject *> &contexts)
{
    contexts.insert("files", this->context);
}

void    Plugin::onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type)
{
    if (type != LightBird::IOnDeserialize::IDoDeserializeHeader)
        return ;
    // Sets the name of the context based on the URL
    QString url = client.getRequest().getUri().toString(QUrl::RemoveQuery | QUrl::StripTrailingSlash);
    client.getContexts().removeAll("files");
    if (url.startsWith("/f/") || url == "/f")
        client.getContexts() << "files";
}

bool    Plugin::doExecution(LightBird::IClient &client)
{
    LightBird::IRequest &request = client.getRequest();
    LightBird::IResponse &response = client.getResponse();
    QString path = LightBird::cleanPath(request.getUri().toString(QUrl::RemoveQuery | QUrl::StripTrailingSlash)).remove(0, 2);
    LightBird::TableDirectories directory;
    LightBird::TableDirectories subDir;
    LightBird::TableFiles file;
    QString title = "LightBird";
    QString body;
    QString html;

    response.getHeader().insert("server", "LightBird");
    response.getHeader().insert("cache-control", "no-cache");
    // Authenticates the client
    if (!client.getAccount() && !this->_authenticate(client, request, response))
        return (true);
    // The URI represents a file
    if (file.setIdFromVirtualPath(path))
    {
        response.getHeader().insert("server", "LightBird");
        response.setType(this->_getMime(file.getFullPath()));
        response.getContent().setStorage(LightBird::IContent::FILE, file.getFullPath());
    }
    // Displays the content of a directory
    else if (directory.cd(path))
    {
        // Page title
        if (!directory.getId().isEmpty())
            title = directory.getName() + " - LightBird";
        // Path
        /*path.clear();
        QStringListIterator p(directory.getVirtualPath().split('/'));
        while (p.hasNext())
        {
            path += p.next() + "/";
            body += Plugin::link.arg(path, p.peekPrevious()) + "/";
        }
        if (!body.isEmpty())
            body += "<br />";*/
        body += "<table>";
        body += "<tr><th>Name</th><th class=\"size\">Size</th><th class=\"date\">Last modification</th></tr>";
        // Parent directory
        subDir.setId(directory.getIdDirectory());
        if (directory)
        {
            body += "<tr>";
            body += "<td>" + QString(Plugin::link).replace("/f/", "/f").arg(subDir.getVirtualPath((bool)subDir), "..") + "</td>";
            body += "<td class=\"size\"></td>";
            body += "<td class=\"date\">" + subDir.getModified().toString("dd/MM/yyyy hh:mm:ss") + "</td>";
            body += "</tr>";
        }
        // Subdirectories
        QStringListIterator d(directory.getDirectories());
        while (d.hasNext())
        {
            subDir.setId(d.next());
            body += "<tr>";
            body += "<td>" + Plugin::link.arg(subDir.getVirtualPath(), subDir.getName()) + "</td>";
            body += "<td class=\"size\"></td>";
            body += "<td class=\"date\">" + subDir.getModified().toString("dd/MM/yyyy hh:mm:ss") + "</td>";
            body += "</tr>";
        }
        // Files
        QStringListIterator f(directory.getFiles());
        while (f.hasNext())
        {
            file.setId(f.next());
            body += "<tr>";
            body += "<td>" + Plugin::link.arg(file.getVirtualPath(false, true), file.getName()) + "</td>";
            body += "<td class=\"size\">" + this->_size(QFileInfo(file.getFullPath()).size()) + "</td>";
            body += "<td class=\"date\">" + file.getModified().toString("dd/MM/yyyy hh:mm:ss") + "</td>";
            body += "<tr>";
        }
        body += "</table>";
    }
    // The file or the directory does not exists
    else
    {
        response.setCode(404);
        response.setMessage("Not Found");
        body = "File not found.";
    }
    // We have to send a HTML response
    if (!body.isEmpty())
    {
        html = this->content.arg(title, body);
        response.getHeader().insert("content-type", "text/html");
        response.getContent().setData(html.toLatin1());
    }
    return (true);
}

bool    Plugin::_authenticate(LightBird::IClient &client, LightBird::IRequest &request, LightBird::IResponse &response)
{
    QString password = QByteArray::fromBase64(QString(request.getHeader().value("authorization")).remove("Basic ").toLatin1());
    QString name = password.left(password.indexOf(':'));

    password = password.right(password.size() - name.size() - 1);
    if (name.isEmpty() || !client.getAccount().setIdFromNameAndPassword(name, password))
    {
        response.setCode(401);
        response.setMessage("Unauthorized");
        response.getHeader().insert("WWW-Authenticate", "Basic realm=\"LightBird\"");
        return (false);
    }
    return (true);
}

QString             Plugin::_getMime(const QString &file)
{
    QList<void *>   extensions;
    QString         result = DEFAULT_CONTENT_TYPE;

    extensions = this->api->extensions().get("IMime");
    if (!extensions.isEmpty())
        result = static_cast<LightBird::IMime *>(extensions.first())->getMime(file);
    this->api->extensions().release(extensions);
    return (result);
}

QString Plugin::_size(quint64 s)
{
    QString result = QString::number(s);
    double  size = s;
    int     unit = (result.size() - 1) / 3;

    if (!s)
        return ("0 B");
    size = size / qPow(1024, (result.size() - 1) / 3);
    result = QString::number(size).replace(".", ",").left(4);
    int c = result.indexOf(",");
    if (c == 3)
        result = result.left(3);
    if (unit == 0)
        result += " B";
    else if (unit == 1)
        result += " KB";
    else if (unit == 2)
        result += " MB";
    else if (unit == 3)
        result += " GB";
    else if (unit == 4)
        result += " TB";
    return (result);
}
