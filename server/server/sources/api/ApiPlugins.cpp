#include "Log.h"
#include "Plugins.hpp"
#include "ApiPlugins.h"

ApiPlugins      *ApiPlugins::_instance = NULL;

ApiPlugins      *ApiPlugins::instance(QObject *parent)
{
    if (ApiPlugins::_instance == NULL)
        ApiPlugins::_instance = new ApiPlugins(parent);
    return (ApiPlugins::_instance);
}

ApiPlugins::ApiPlugins(QObject *parent) : QObject(parent)
{
}

ApiPlugins::~ApiPlugins()
{
}

QSharedPointer<LightBird::IFuture<bool> > ApiPlugins::load(const QString &id)
{
    return (QSharedPointer<LightBird::IFuture<bool> >(new Future<bool>(Plugins::instance()->load(id))));
}

QSharedPointer<LightBird::IFuture<bool> > ApiPlugins::unload(const QString &id)
{
    return (QSharedPointer<LightBird::IFuture<bool> > (new Future<bool>(Plugins::instance()->unload(id))));
}

QSharedPointer<LightBird::IFuture<bool> > ApiPlugins::install(const QString &id)
{
    return (QSharedPointer<LightBird::IFuture<bool> > (new Future<bool>(Plugins::instance()->install(id))));
}

QSharedPointer<LightBird::IFuture<bool> > ApiPlugins::uninstall(const QString &id)
{
    return (QSharedPointer<LightBird::IFuture<bool> > (new Future<bool>(Plugins::instance()->uninstall(id))));
}

LightBird::IMetadata ApiPlugins::getMetadata(const QString &id) const
{
    return (Plugins::instance()->getMetadata(id));
}

LightBird::IPlugins::State ApiPlugins::getState(const QString &id)
{
    return (Plugins::instance()->getState(id));
}

QStringList ApiPlugins::getPlugins()
{
    return (Plugins::instance()->getPlugins());
}

QStringList ApiPlugins::getLoadedPlugins()
{
    return (Plugins::instance()->getLoadedPlugins());
}

QStringList ApiPlugins::getUnloadedPlugins()
{
    return (Plugins::instance()->getUnloadedPlugins());
}

QStringList ApiPlugins::getInstalledPlugins()
{
    return (Plugins::instance()->getInstalledPlugins());
}

QStringList ApiPlugins::getUninstalledPlugins()
{
    return (Plugins::instance()->getUninstalledPlugins());
}
