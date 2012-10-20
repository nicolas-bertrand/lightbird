#include <QtPlugin>

#include "IMime.h"
#include "LightBird.h"
#include "Plugin.h"

const QString Plugin::link = "<a href=\"/f/%1\">%2</a><br />";

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
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

bool    Plugin::doExecution(LightBird::IClient &client)
{
    QString path = LightBird::cleanPath(client.getRequest().getUri().toString(QUrl::RemoveQuery | QUrl::StripTrailingSlash)).remove(0, 2);
    LightBird::TableDirectories directory;
    LightBird::TableDirectories subDir;
    LightBird::TableFiles file;
    QString body;
    QString html;
    LightBird::IContent &content = client.getResponse().getContent();

    client.getResponse().getHeader().insert("server", "LightBird");
    client.getResponse().getHeader().insert("cache-control", "no-cache");
    if (!client.getAccount())
    {
        if (!client.getRequest().getHeader().contains("authorization"))
        {
            client.getResponse().setCode(401);
            client.getResponse().setMessage("Unauthorized");
            client.getResponse().getHeader().insert("WWW-Authenticate", "Basic realm=\"LightBird\"");
            return (true);
        }
        else
        {
            QList<QString> auth = QString(QByteArray::fromBase64(QString(client.getRequest().getHeader().value("authorization")).remove("Basic ").toAscii())).split(':');
            if (auth.size() != 2 || !client.getAccount().setIdFromNameAndPassword(auth.at(0), auth.at(1)))
            {
                client.getResponse().setCode(401);
                client.getResponse().setMessage("Unauthorized");
                client.getResponse().getHeader().insert("WWW-Authenticate", "Basic realm=\"LightBird\"");
                return (true);
            }
        }
    }
    if (file.setIdFromVirtualPath(path))
    {
        client.getResponse().getHeader().insert("server", "LightBird");
        client.getResponse().setType(this->_getMime(file.getFullPath()));
        content.setStorage(LightBird::IContent::FILE, file.getFullPath());
    }
    else if (directory.cd(path))
    {
        subDir.setId(directory.getIdDirectory());
        if (directory)
            body += Plugin::link.arg(subDir.getVirtualPath(), "..");
        QStringListIterator d(directory.getDirectories());
        while (d.hasNext())
        {
            subDir.setId(d.next());
            body += Plugin::link.arg(subDir.getVirtualPath(), subDir.getName());
        }
        QStringListIterator f(directory.getFiles());
        while (f.hasNext())
        {
            file.setId(f.next());
            body += Plugin::link.arg(file.getVirtualPath(false, true), file.getName());
        }
    }
    else
    {
        client.getResponse().setCode(404);
        client.getResponse().setMessage("Not Found");
        body = "File not found.";
    }
    if (!body.isEmpty())
    {
        html = this->content.arg("LightBird", body);
        client.getResponse().getContent().setContent(html.toAscii());
    }
    return (true);
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

Q_EXPORT_PLUGIN2(Plugin, Plugin)
