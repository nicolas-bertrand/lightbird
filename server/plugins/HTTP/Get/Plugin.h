#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IPlugin.h"
# include "IResources.h"
# include "IDoExecution.h"

class Plugin : public QObject,
               public Streamit::IPlugin,
               public Streamit::IResources,
               public Streamit::IDoExecution
{
    Q_OBJECT
    Q_INTERFACES(Streamit::IPlugin Streamit::IResources Streamit::IDoExecution)

public:
    Plugin();
    ~Plugin();

    // IPlugin
    bool    onLoad(Streamit::IApi *api);
    void    onUnload();
    bool    onInstall(Streamit::IApi *api);
    void    onUninstall(Streamit::IApi *api);
    void    getMetadata(Streamit::IMetadata &metadata) const;

    // IResources
    QString getResourcesPath();

    // Execution
    bool    doExecution(Streamit::IClient &client);

private:
    Streamit::IApi  *api;
};

#endif // PLUGIN_H
