#ifndef CLIENTHANDLER_H
# define CLIENTHANDLER_H

# include <QHash>
# include <QHostAddress>
# include <QMutex>
# include <QWaitCondition>

# include "ISessions.h"

# include "Commands.h"

/// @brief Manages the clients on the control and data connections.
class ClientHandler
{
public:
    ClientHandler(LightBird::IApi &api);
    ~ClientHandler();

    bool    onConnect(LightBird::IClient &client);
    bool    onDataConnect(LightBird::IClient &client);
    bool    doControlExecute(LightBird::IClient &client);
    bool    doDataExecute(LightBird::IClient &client);
    void    onDataDestroy(LightBird::IClient &client);

private:
    Commands::Result _prepareTransferMethod(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

    LightBird::IApi  &api;
    Commands         *commands;
    QMutex           mutex; ///< Makes the class thread safe.
    /// The list of the clients that are waiting to be associated with a control connection in passive mode.
    QList<QPair<QHostAddress, QString> > passiveClients;
    /// The list of the clients that are waiting the control connection to be ready before the start of the unserialization.
    QHash<QString, QWaitCondition *> wait;

};

#endif // CLIENTHANDLER_H
