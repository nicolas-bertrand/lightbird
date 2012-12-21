#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IPlugin.h"

class Plugin : public QObject,
               public LightBird::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Example.Basic")
    Q_INTERFACES(LightBird::IPlugin)

public:
    Plugin();
    ~Plugin();

    // LightBird::IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

private:
    LightBird::IApi *api;
};

#endif // PLUGIN_H
