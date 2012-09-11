#ifndef CLIENTHANDLER_H
# define CLIENTHANDLER_H

# include <QString>

# include "ISessions.h"

# include "Execute.h"

class ClientHandler
{
public:
    ClientHandler(LightBird::IApi *api);
    ~ClientHandler();

    bool    onConnect(LightBird::IClient *client);
    bool    onDataConnect(LightBird::IClient *client);
    bool    doControlExecute(LightBird::IClient *client);
    bool    doDataExecute(LightBird::IClient *client);

private:
    void    _sendControlMessage(QString id, Execute::MethodResult message);
    Execute::MethodResult   _prepareTransferMethod(QString verb, QString parameter, LightBird::Session session);

    LightBird::IApi *api;
    Execute         *execute;
    QMap<QString, Execute::ControlMethod> controlMethods;
    QMap<QString, QPair<bool, Execute::TransferMethod> > transferMethods; ///< The bool of the pair is true if the methods sends data on transfer connection, false if it receives some
};


#endif // CLIENTHANDLER_H
