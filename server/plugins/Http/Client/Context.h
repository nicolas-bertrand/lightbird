#ifndef PLUGIN_CONTEXT_H
# define PLUGIN_CONTEXT_H

# include <QObject>

# include "IOnDeserialize.h"
# include "IDoExecution.h"
# include "IOnSerialize.h"
# include "IOnFinish.h"
# include "IOnDisconnect.h"
# include "IOnDestroy.h"

class Plugin;

/// @brief The interfaces of this class are called when the context is valid,
/// and calls their equivalent in Plugin.
class Context : public QObject,
                public LightBird::IOnDeserialize,
                public LightBird::IDoExecution,
                public LightBird::IOnSerialize,
                public LightBird::IOnFinish,
                public LightBird::IOnDisconnect,
                public LightBird::IOnDestroy
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IOnDeserialize
                 LightBird::IDoExecution
                 LightBird::IOnSerialize
                 LightBird::IOnFinish
                 LightBird::IOnDisconnect
                 LightBird::IOnDestroy)

public:
    void    onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type);
    bool    doExecution(LightBird::IClient &client);
    bool    onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type);
    void    onFinish(LightBird::IClient &client);
    bool    onDisconnect(LightBird::IClient &client, bool fatal);
    void    onDestroy(LightBird::IClient &client);
};

#endif // PLUGIN_CONTEXT_H
