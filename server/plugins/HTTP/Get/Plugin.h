#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IPlugin.h"
# include "IResources.h"
# include "IDoExecution.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IResources,
               public LightBird::IDoExecution
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin LightBird::IResources LightBird::IDoExecution)

public:
    Plugin();
    ~Plugin();

    // IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // IResources
    QString getResourcesPath();

    // Execution
    bool    doExecution(LightBird::IClient &client);

private:
    LightBird::IApi *api;
};

#endif // PLUGIN_H
