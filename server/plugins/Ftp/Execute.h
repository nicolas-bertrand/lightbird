#ifndef EXECUTE_H
# define EXECUTE_H

# include <QPair>
# include <QString>
# include "IApi.h"
# include "ISessions.h"
# include "IRequest.h"
# include "IResponse.h"

class Execute
{
    public:
        Execute(LightBird::IApi *api);
        ~Execute();

        enum TransferMode {
            TransferModeNone,
            TransferModePort,
            TransferModePasv
        };

        struct Upload {
            QTemporaryFile *file;
            QString name;
            QString parent;
        };

        typedef QPair<int, QString> MethodResult; // The code and the message associated of a response on control connetcion

        // A control method takes in a FTP command and the corresponding parmater string (possibly empty),
        // as well as the session corresponding to the connection,
        // and returns a pair with a code and a message to send on the control connection
        typedef MethodResult (Execute::*ControlMethod)(QString parameter, LightBird::Session session);

        // A transfer method is similar to a control method
        // However if either uses a IRequest or fills in a IResponse with the contents of the data connection,
        // depending if it is  a in or out transfer
        typedef MethodResult (Execute::*TransferMethod)(QString parameter, LightBird::Session session, LightBird::IRequest *request, LightBird::IResponse *response);


        // Control methods
        MethodResult doGreeting (QString parameter, LightBird::Session session);
        MethodResult doUser     (QString parameter, LightBird::Session session);
        MethodResult doPass     (QString parameter, LightBird::Session session);
        MethodResult doSyst     (QString parameter, LightBird::Session session);
        MethodResult doPwd      (QString parameter, LightBird::Session session);
        MethodResult doType     (QString parameter, LightBird::Session session);
        MethodResult doPort     (QString parameter, LightBird::Session session);
        MethodResult doCwd      (QString parameter, LightBird::Session session);
        MethodResult doCdup     (QString parameter, LightBird::Session session);


        // Transfer methods
        MethodResult doList(QString parameter, LightBird::Session session, LightBird::IRequest *request, LightBird::IResponse *response);
        MethodResult doRetr(QString parameter, LightBird::Session session, LightBird::IRequest *request, LightBird::IResponse *response);
        MethodResult doStor(QString parameter, LightBird::Session session, LightBird::IRequest *request, LightBird::IResponse *response);

    private:
        LightBird::IApi *api;
};

#endif // EXECUTE_H
