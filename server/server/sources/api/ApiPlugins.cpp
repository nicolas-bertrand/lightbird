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
    Log::trace("ApiPlugins destroyed!", "ApiPlugins", "~ApiPlugins");
}

QSharedPointer<Streamit::IFuture<bool> > ApiPlugins::load(const QString &id)
{
    Streamit::IPlugins::State   state;

    // If the plugin is not installed
    state = Plugins::instance()->getState(id);
    if (state == Streamit::IPlugins::UNINSTALLED ||
        state == Streamit::IPlugins::UNKNOW)
        Plugins::instance()->install(id);
    // Creates a shared pointer that will automatically delete the Future
    return (QSharedPointer<Streamit::IFuture<bool> >(new Future<bool>(Plugins::instance()->load(id))));
}

QSharedPointer<Streamit::IFuture<bool> > ApiPlugins::unload(const QString &id)
{
    return (QSharedPointer<Streamit::IFuture<bool> > (new Future<bool>(Plugins::instance()->unload(id))));
}

QSharedPointer<Streamit::IFuture<bool> > ApiPlugins::install(const QString &id)
{
    return (QSharedPointer<Streamit::IFuture<bool> > (new Future<bool>(Plugins::instance()->install(id))));
}

QSharedPointer<Streamit::IFuture<bool> > ApiPlugins::uninstall(const QString &id)
{
    return (QSharedPointer<Streamit::IFuture<bool> > (new Future<bool>(Plugins::instance()->uninstall(id))));
}

Streamit::IPlugins::State ApiPlugins::getState(const QString &id)
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

QStringList ApiPlugins::getInstalledPlugins()
{
    return (Plugins::instance()->getInstalledPlugins());
}

QStringList ApiPlugins::getUninstalledPlugins()
{
    return (Plugins::instance()->getUninstalledPlugins());
}
