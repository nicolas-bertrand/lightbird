#ifndef ENGINESERVER_H
# define ENGINESERVER_H

# include "Engine.h"

/// @brief The role of the server Engine is to execute the request of a client
/// and generate a response, by calling the interfaces of the Api that are
/// implemented by the plugins. The operations of the Engine are divided into
/// multiple tasks that are executed by the ThreadPool. This way, the processing
/// can be stopped at any time, and hundreds of engines can run together.
class EngineServer : public Engine
{
    Q_OBJECT

public:
    EngineServer(Client &client);
    ~EngineServer();

    bool    run();
    /// @brief Sends a response without waiting for a request. Bypass the
    /// deserialize api, and call directly IOnDeserialize followed by IDoExecution.
    /// @param protocol : The protocol used to communicate with the client.
    /// @param informations : The informations of the request.
    /// @return False if the engine is not idle.
    bool    send(const QString &protocol, const QVariantMap &informations);
    /// @brief Returns true if the engine has just been cleared and no data has
    /// been received yet.
    bool    isIdle();

private:
    EngineServer(const EngineServer &);
    EngineServer &operator=(const EngineServer &);

    void    _clear();

private slots:
    // This methods calls the interfaces implemented by the plugins,
    // in order to execute the request and generate a response.
    bool    _onProtocol();
    bool    _doDeserializeHeader();
    bool    _doDeserializeContent();
    bool    _doDeserializeTrailer();
    bool    _doExecution();
    bool    _onExecution();
    bool    _doSerializeHeader();
    bool    _doSerializeContent();
    bool    _doSerializeTrailer();
    void    _onFinish();

private:
    /// @brief A simple pointer to method.
    /// @return True while the engine has enough data to run.
    typedef bool (EngineServer::*Method)();

    Method      state;          ///< A pointer to method to the next step of the processing of the data.
    bool        needResponse;   ///< If the request of the client needs a response.
    QStringList protocolUnknow; ///< List the plugins that doesn't know the protocol of the request.
    bool        idle;           ///< True if the engine has just been cleared, and no data has been received yet.
};

#endif // ENGINESERVER_H
