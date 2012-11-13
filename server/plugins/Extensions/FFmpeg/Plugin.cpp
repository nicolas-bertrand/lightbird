#include <QtPlugin>

#include "Plugin.h"

Plugin  *Plugin::instance = NULL;

Plugin::Plugin()
{
    this->identify = NULL;
    this->preview = NULL;
    this->instance = this;
}

Plugin::~Plugin()
{
    this->instance = NULL;
}
#include <iostream>
bool    Plugin::onLoad(LightBird::IApi *api)
{
    AVOutputFormat *format = NULL;

    this->api = api;
    // Initializes FFmpeg
    avcodec_register_all();
    av_register_all();
    avfilter_register_all();
    this->_loadConfiguration();
    // Gets the registered formats
    while ((format = av_oformat_next(format)))
        this->formats << format->name;
    this->formats.removeDuplicates();
    // Loads the extensions
    this->identify = new Identify(this->api);
    this->preview = new Preview(this->api);
    return (true);
}

void    Plugin::onUnload()
{
    delete this->identify;
    delete this->preview;
    this->identify = NULL;
    this->preview = NULL;
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
    metadata.name = "FFmpeg";
    metadata.brief = "Implements the IVideo and IAudio extensions using FFmpeg.";
    metadata.description = "Implements the IVideo and IAudio extensions using FFmpeg.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

QStringList Plugin::getExtensionsNames()
{
    return (QStringList() << "IIdentify" << "IPreview" << "IVideo");
}

void    *Plugin::getExtension(const QString &name)
{
    if (name == "IIdentify")
        return (dynamic_cast<LightBird::IIdentify *>(this->identify));
    else if (name == "IPreview")
        return (dynamic_cast<LightBird::IPreview *>(this->preview));
    else if (name == "IVideo")
    {
        Video *video = new Video(this->api, this->formats);
        this->mutex.lock();
        this->videos.push_back(QSharedPointer<Video>(video));
        this->mutex.unlock();
        return (dynamic_cast<LightBird::IVideo *>(video));
    }
    return (NULL);
}

void    Plugin::releaseExtension(const QString &name, void *extension)
{
    this->mutex.lock();
    if (name == "IVideo")
    {
        QMutableListIterator<QSharedPointer<Video> > it(this->videos);
        while (it.hasNext())
            if (dynamic_cast<LightBird::IVideo *>(it.next().data()) == extension)
            {
                it.remove();
                break;
            }
    }
    this->mutex.unlock();
}

int     Plugin::avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
{
    int result;

    Plugin::instance->mutex.lock();
    result = ::avcodec_open2(avctx, codec, options);
    Plugin::instance->mutex.unlock();
    return (result);
}

void    Plugin::_loadConfiguration()
{
    QMap<QString, int>  logLevels;
    QString             logLevel;

    logLevels.insert("QUIET"  , AV_LOG_QUIET);
    logLevels.insert("PANIC"  , AV_LOG_PANIC);
    logLevels.insert("FATAL"  , AV_LOG_FATAL);
    logLevels.insert("ERROR"  , AV_LOG_ERROR);
    logLevels.insert("WARNING", AV_LOG_WARNING);
    logLevels.insert("INFO"   , AV_LOG_INFO);
    logLevels.insert("VERBOSE", AV_LOG_VERBOSE);
    logLevels.insert("DEBUG"  , AV_LOG_DEBUG);
    if ((logLevel = this->api->configuration(true).get("av_log_level")).isEmpty() || !logLevels.contains(logLevel))
        logLevel = "FATAL";
    av_log_set_level(logLevels.value(logLevel));
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
