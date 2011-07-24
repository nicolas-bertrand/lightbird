#include "ApiPlugins.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Server.h"

ApiPlugins::ApiPlugins(QObject *parent) : QObject(parent)
{
    Log::trace("ApiPlugins created", "ApiPlugins", "ApiPlugins");
}

ApiPlugins::~ApiPlugins()
{
    Log::trace("ApiPlugins destroyed!", "ApiPlugins", "~ApiPlugins");
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

LightBird::IPlugins::State ApiPlugins::getState(const QString &id) const
{
    return (Plugins::instance()->getState(id));
}

QStringList ApiPlugins::getPlugins() const
{
    return (Plugins::instance()->getPlugins());
}

QStringList ApiPlugins::getLoadedPlugins() const
{
    return (Plugins::instance()->getLoadedPlugins());
}

QStringList ApiPlugins::getUnloadedPlugins() const
{
    return (Plugins::instance()->getUnloadedPlugins());
}

QStringList ApiPlugins::getInstalledPlugins() const
{
    return (Plugins::instance()->getInstalledPlugins());
}

QStringList ApiPlugins::getUninstalledPlugins() const
{
    return (Plugins::instance()->getUninstalledPlugins());
}

ApiPlugins  *ApiPlugins::instance()
{
    return (Server::instance().getApiPlugins());
}
