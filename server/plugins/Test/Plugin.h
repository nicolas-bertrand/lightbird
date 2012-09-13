#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IEvent.h"
# include "IPlugin.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IEvent
{
    Q_OBJECT
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
};

// Throws an exception if the assertion is false
# define ASSERT(a)\
if (!(a))\
{\
    QMap<QString, QString> properties;\
    properties.insert("line", QString::number(__LINE__));\
    throw (properties);\
}\
else (void)0

#endif // PLUGIN_H
