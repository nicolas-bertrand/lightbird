#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IPlugin.h"
# include "ITimer.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::ITimer
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::ITimer)

public:
    Plugin();
    ~Plugin();

    // LightBird::IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // LightBird::ITimer
    bool    timer(const QString &name);

private:
    LightBird::IApi *api;
};

#endif // PLUGIN_H
