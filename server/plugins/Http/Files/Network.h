#ifndef NETWORK_H
# define NETWORK_H

# include <QObject>

# include "IDoExecution.h"

class Plugin;

/// @brief The interfaces of this class are called when the context is valid,
/// and calls their equivalent in Plugin.
class Network : public QObject,
                public LightBird::IDoExecution
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IDoExecution)

public:
    Network(Plugin *plugin);

    bool    doExecution(LightBird::IClient &client);

private:
    Plugin  *plugin; ///< The instance of the plugin.
};

#endif // NETWORK_H
