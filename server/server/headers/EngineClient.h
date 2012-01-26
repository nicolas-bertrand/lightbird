#ifndef ENGINECLIENT_H
# define ENGINECLIENT_H

# include "Engine.h"

/// @brief The role of the client Engine is to generate a request and execute
/// the response of the client, by calling the interfaces of the Api that are
/// implemented by the plugins. The operations of the Engine are divided into
/// multiple tasks that are executed by the ThreadPool. This way, the processing
/// can be stopped at any time, and hundreds of engines can run together.
class EngineClient : public Engine
{
    Q_OBJECT

public:
    EngineClient(Client &client);
    ~EngineClient();

    bool    run();
    /// @brief Adds a new request to send.
    /// @param id : The id of the plugin that wants to send the request.
    /// @return True if the engine is ready to run and send a new request.
    bool    send(const QString &id, const QString &protocol);

private:
    EngineClient(const EngineClient &);
    EngineClient &operator=(const EngineClient &);

    void    _clear();

private slots:
    // This methods calls the interfaces implemented by the plugins,
    // in order to generate a request and execute the response.
    bool    _doSend();
    bool    _onSend();
    bool    _doSerializeHeader();
    bool    _doSerializeContent();
    bool    _doSerializeFooter();
    bool    _doUnserializeHeader();
    bool    _doUnserializeContent();
    bool    _doUnserializeFooter();
    bool    _doExecution();
    bool    _onExecution();
    bool    _onFinish();

private:
    /// @brief A simple pointer to method.
    /// @return True while the engine has enough data to run.
    typedef bool (EngineClient::*Method)();

    ///< A pointer to method to the next step of the processing of the data.
    Method  state;
    /// Stores the list of the requests that the plugins want to send.
    QList<QPair<QString, QString> > requests;
};

#endif // ENGINECLIENT_H
