#ifndef CONTROL_H
# define CONTROL_H

# include <QObject>

# include "IApi.h"
# include "IClient.h"
# include "IDoDeserializeContent.h"
# include "IDoExecution.h"
# include "IOnResume.h"
# include "IOnExecution.h"
# include "IDoSerializeContent.h"
# include "IOnFinish.h"
# include "IOnDisconnect.h"
# include "IOnDestroy.h"

# include "Commands.h"

/// @brief Manages the control connection.
class Control : public QObject,
                public LightBird::IDoDeserializeContent,
                public LightBird::IDoExecution,
                public LightBird::IOnResume,
                public LightBird::IOnExecution,
                public LightBird::IDoSerializeContent,
                public LightBird::IOnFinish,
                public LightBird::IOnDisconnect,
                public LightBird::IOnDestroy
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IDoDeserializeContent
                 LightBird::IDoExecution
                 LightBird::IOnResume
                 LightBird::IOnExecution
                 LightBird::IDoSerializeContent
                 LightBird::IOnFinish
                 LightBird::IOnDisconnect
                 LightBird::IOnDestroy)

public:
    Control(LightBird::IApi *api);
    ~Control();

    bool    onConnect(LightBird::IClient &client);
    bool    doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used);
    bool    doExecution(LightBird::IClient &client);
    void    onResume(LightBird::IClient &client, bool timeout);
    bool    onExecution(LightBird::IClient &client);
    bool    doSerializeContent(LightBird::IClient &client, QByteArray &data);
    void    onFinish(LightBird::IClient &client);
    bool    onDisconnect(LightBird::IClient &client, bool fatal);
    void    onDestroy(LightBird::IClient &client);

private:
    Commands::Result _prepareTransferCommand(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

    LightBird::IApi  *api;
};

#endif // CONTROL_H
