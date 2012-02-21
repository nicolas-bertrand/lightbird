#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IPlugin.h"
# include "IOnUnserialize.h"
# include "IOnSerialize.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnUnserialize,
               public LightBird::IOnSerialize
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin LightBird::IOnUnserialize LightBird::IOnSerialize)

public:
    Plugin();
    ~Plugin();

    // IPlugin
    bool        onLoad(LightBird::IApi *api);
    void        onUnload();
    bool        onInstall(LightBird::IApi *api);
    void        onUninstall(LightBird::IApi *api);
    void        getMetadata(LightBird::IMetadata &metadata) const;

    // Unserialize / Serialize
    void        onUnserialize(LightBird::IClient &client, LightBird::IOnUnserialize::Unserialize type);
    bool        onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type);

private:
    QByteArray  _serializer(const QVariant &toAnalyze);
    QByteArray  _replace(QByteArray str);

    LightBird::IApi *api; ///< The LightBird's Api.
};

#endif // PLUGIN_H
