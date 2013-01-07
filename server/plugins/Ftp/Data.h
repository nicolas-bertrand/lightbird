#ifndef DATA_H
# define DATA_H

# include <QObject>

# include "IApi.h"
# include "IClient.h"
# include "IOnResume.h"
# include "IDoDeserializeHeader.h"
# include "IDoDeserializeContent.h"
# include "IDoExecution.h"
# include "IDoSend.h"
# include "IOnSerialize.h"
# include "IDoSerializeContent.h"
# include "IOnFinish.h"
# include "IOnDisconnect.h"
# include "IOnDestroy.h"

/// @brief Manages the data connection.
class Data : public QObject,
             public LightBird::IOnResume,
             public LightBird::IDoDeserializeHeader,
             public LightBird::IDoDeserializeContent,
             public LightBird::IDoExecution,
             public LightBird::IDoSend,
             public LightBird::IOnSerialize,
             public LightBird::IDoSerializeContent,
             public LightBird::IOnFinish,
             public LightBird::IOnDisconnect,
             public LightBird::IOnDestroy
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IOnResume
                 LightBird::IDoDeserializeHeader
                 LightBird::IDoDeserializeContent
                 LightBird::IDoExecution
                 LightBird::IDoSend
                 LightBird::IOnSerialize
                 LightBird::IDoSerializeContent
                 LightBird::IOnFinish
                 LightBird::IOnDisconnect
                 LightBird::IOnDestroy)

public:
    Data(LightBird::IApi *api);
    ~Data();

    bool    onConnect(LightBird::IClient &client);
    void    onResume(LightBird::IClient &client, bool timeout);
    /// @brief Called when the client is sending data to us, in order to initiate the upload.
    bool    doDeserializeHeader(LightBird::IClient &client, const QByteArray &data, quint64 &used);
    bool    doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used);
    /// @brief Called when data are being sent in passive mode (SERVER mode).
    bool    doExecution(LightBird::IClient &client);
    /// @brief Called when data are going to be sent in active mode (CLIENT mode).
    bool    doSend(LightBird::IClient &client);
    /// @return False because the response to the client is not needed in CLIENT mode.
    bool    onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type);
    bool    doSerializeContent(LightBird::IClient &client, QByteArray &data);
    /// @brief Disconnects the data connection when it is finished.
    void    onFinish(LightBird::IClient &client);
    bool    onDisconnect(LightBird::IClient &client, bool fatal);
    void    onDestroy(LightBird::IClient &client);

private:
    bool    _execute(LightBird::IClient &client);

    LightBird::IApi *api;
};

#endif // DATA_H
