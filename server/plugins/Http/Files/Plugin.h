#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IDoExecution.h"
# include "IPlugin.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IDoExecution
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IDoExecution)

public:
    Plugin();
    ~Plugin();

    // LightBird::IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    bool    doExecution(LightBird::IClient &client);

private:
    QString _getMime(const QString &file);

    LightBird::IApi *api;
    QString         content;
    static const QString link;
};

#endif // PLUGIN_H
