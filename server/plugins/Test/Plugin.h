#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IEvent.h"
# include "IPlugin.h"
# include "ITest.h"

class Plugin
    : public QObject
    , public LightBird::IPlugin
    , public LightBird::IEvent
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Test")
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IEvent)

public:
    Plugin();
    ~Plugin();

    // LightBird::IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // LightBird::IEvent
    void    event(const QString &event, const QVariant &property = QVariant());

private:
    LightBird::IApi *api;
    QList<QPair<QString, ITest *> > tests;
    bool shutdown; ///< If true the server is shut down after the tests.
};

#endif // PLUGIN_H
