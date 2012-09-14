#ifndef CLIENTHANDLER_H
# define CLIENTHANDLER_H

# include <QString>

# include "ISessions.h"

# include "Commands.h"

class ClientHandler
{
public:
    ClientHandler(LightBird::IApi *api);
    ~ClientHandler();

    bool    onConnect(LightBird::IClient &client);
    bool    onDataConnect(LightBird::IClient &client);
    bool    doControlExecute(LightBird::IClient &client);
    bool    doDataExecute(LightBird::IClient &client);

private:
    void    _sendControlMessage(QString id, Commands::Result message);
    Commands::Result _prepareTransferMethod(QString command, QString parameter, LightBird::Session session);

    LightBird::IApi  *api;
    Commands         *commands;
};


#endif // CLIENTHANDLER_H
