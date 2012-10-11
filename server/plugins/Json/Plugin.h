#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IPlugin.h"
# include "IOnDeserialize.h"
# include "IOnSerialize.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnDeserialize,
               public LightBird::IOnSerialize
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin LightBird::IOnDeserialize LightBird::IOnSerialize)

public:
    Plugin();
    ~Plugin();

    // IPlugin
    bool        onLoad(LightBird::IApi *api);
    void        onUnload();
    bool        onInstall(LightBird::IApi *api);
    void        onUninstall(LightBird::IApi *api);
    void        getMetadata(LightBird::IMetadata &metadata) const;

    // Deserialize / Serialize
    void        onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type);
    bool        onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type);

private:
    QByteArray  _serializer(const QVariant &toAnalyze);
    QByteArray  _replace(QByteArray str);

    LightBird::IApi *api; ///< The LightBird's Api.
};

#endif // PLUGIN_H
