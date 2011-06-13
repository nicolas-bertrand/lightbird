#ifndef ENGINECLIENT_H
# define ENGINECLIENT_H

# include "Engine.h"

# include "IOnSerialize.h"
# include "IOnUnserialize.h"

/// @brief The role of this Engine is to send a request to a client ans execute,
/// its response by calling the interfaces of the Api that are implemented by the
/// plugins. The Engine uses massively the signals/slots features of Qt. This way,
/// the execution of a request can be stopped at any time, if the client is disconnected.
class EngineClient : public Engine
{
    Q_OBJECT

public:
    EngineClient(Client &client, QObject *parent = 0);
    ~EngineClient();

    void    read(QByteArray &data);
    void    send(const QString &id, const QString &protocol);
    bool    isRunning();
    LightBird::IRequest  &getRequest();
    LightBird::IResponse &getResponse();

private:
    EngineClient(const EngineClient &context);
    EngineClient &operator=(const EngineClient &context);

    /// Stores the list of the requests that the plugins want to send.
    QList<QPair<QString, QString> > requests;

signals:
    // Each signals represents a step of the data flow
    void    doSend();
    void    doSerializeHeader();
    void    doSerializeContent();
    void    doSerializeFooter();
    void    doUnserializeHeader();
    void    doUnserializeContent();
    void    doUnserializeFooter();
    void    doExecution();
    void    onExecution();

private slots:
    // This methods calls the interfaces implemented by the plugin,
    // in order to generate a request and execute the response.
    void    _onRead(QByteArray &data);
    void    _doSend();
    bool    _onSend();
    void    _doSerializeHeader();
    void    _doSerializeContent();
    void    _doSerializeFooter();
    bool    _onSerialize(LightBird::IOnSerialize::Serialize type);
    void    _onWrite(QByteArray &data);
    void    _doUnserializeHeader();
    void    _doUnserializeContent();
    void    _doUnserializeFooter();
    void    _onUnserialize(LightBird::IOnUnserialize::Unserialize type);
    void    _doExecution();
    void    _onExecution();
    void    _onFinish();
};

#endif // ENGINECLIENT_H
