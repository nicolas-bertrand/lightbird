#ifndef ENGINESERVER_H
# define ENGINESERVER_H

# include "IOnSerialize.h"
# include "IOnUnserialize.h"

# include "Engine.h"

/// @brief The role of this server Engine is to execute a request sent by a client,
/// by calling the interfaces of the Api that are implemented by the plugins.
/// The Engine uses massively the signals/slots features of Qt. This way, the
/// execution of a request can be stopped at any time, if the client is disconnected.
/// The final goal of the engine is to generates a response that can be sent to
/// the client.
class EngineServer : public Engine
{
    Q_OBJECT

public:
    EngineServer(Client &client, QObject *parent = 0);
    ~EngineServer();

    void    read(QByteArray &data);
    bool    isRunning();
    LightBird::IRequest  &getRequest();
    LightBird::IResponse &getResponse();

private:
    EngineServer(const EngineServer &context);
    EngineServer &operator=(const EngineServer &context);

signals:
    // Each signals represents a step in the execution of the request
    void    onProtocol();
    void    doUnserializeHeader();
    void    doUnserializeContent();
    void    doUnserializeFooter();
    void    doExecution();
    void    onExecution();
    void    doSerializeHeader();
    void    doSerializeContent();
    void    doSerializeFooter();

private slots:
    // This methods calls the interfaces implemented by the plugin,
    // in order to execute the request and generate a response.
    void    _onRead(QByteArray &data);
    void    _onProtocol();
    void    _doUnserializeHeader();
    void    _doUnserializeContent();
    void    _doUnserializeFooter();
    void    _onUnserialize(LightBird::IOnUnserialize::Unserialize type);
    void    _doExecution();
    void    _onExecution();
    void    _onSerialize(LightBird::IOnSerialize::Serialize type);
    void    _doSerializeHeader();
    void    _doSerializeContent();
    void    _doSerializeFooter();
    void    _onWrite(QByteArray &data);
    void    _onFinish();
};

#endif // ENGINESERVER_H
