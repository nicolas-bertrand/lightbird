#ifndef CLIENT_HANDLER_H
# define CLIENT_HANDLER_H

# include "ISessions.h"
# include "Execute.h"

namespace LightBird
{
    class IApi;
    class IClient;
    class IRequest;
    class IResponse;
}

class QString;

class ClientHandler
{
    public:
        ClientHandler(LightBird::IApi *api);
        ~ClientHandler();
        bool onConnect(LightBird::IClient *client);
        bool onDataConnect(LightBird::IClient *client);
        bool doControlExecute(LightBird::IClient *client);
        bool doDataExecute(LightBird::IClient *client);

    private:
        LightBird::IApi *api;
        Execute *execute;

        QMap<QString,Execute::ControlMethod> controlMethods;
        QMap<QString,QPair<bool,Execute::TransferMethod> > transferMethods; // The bool of the pair is true if the methods sends data on transfer connection, false if it receives some

        void _sendControlMessage(QString id, Execute::MethodResult message);

        Execute::MethodResult _prepareTransferMethod(QString verb, QString parameter, LightBird::Session session);

};


#endif // CLIENT_HANDLER_H
