#include "Plugin.h"

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
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
    metadata.name = "Mime";
    metadata.brief = "Allows to get the mime type of a file based on its extension.";
    metadata.description = "This plugin implements the IMime interface, and returns the mime type of a file based on its extension.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

QStringList Plugin::getExtensionsNames()
{
    return (QStringList() << "IMime");
}

void        *Plugin::getExtension(const QString &name)
{
    if (name == "IMime")
        return (dynamic_cast<LightBird::IMime *>(this));
    return (NULL);
}

void        Plugin::releaseExtension(const QString &, void *)
{
}

QString     Plugin::getMime(const QString &file)
{
    QString                   extension;
    LightBird::IConfiguration *configuration = NULL;

    if (file.contains(".") && (configuration = this->api->configuration(this->api->getPluginPath() + "/Mime.xml")))
    {
        extension = file.right(file.size() - file.lastIndexOf(".") - 1);
        if (!(extension = configuration->get(extension.toLower())).isEmpty())
            return (extension);
    }
    return ("application/octet-stream");
}
