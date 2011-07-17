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
    void    clear();

private:
    EngineServer(const EngineServer &context);
    EngineServer &operator=(const EngineServer &context);

private slots:
    // This methods calls the interfaces implemented by the plugins,
    // in order to execute the request and generate a response.
    bool    _onProtocol();
    bool    _doUnserializeHeader();
    bool    _doUnserializeContent();
    bool    _doUnserializeFooter();
    bool    _doExecution();
    bool    _onExecution();
    bool    _doSerializeHeader();
    bool    _doSerializeContent();
    bool    _doSerializeFooter();
    void    _onFinish();

private:
    /// @brief A simple pointer to method.
    /// @return True while the engine has enough data to run.
    typedef bool (EngineServer::*Method)();

    Method      state;          ///< A pointer to method to the next step of the processing of the data.
    bool        needResponse;   ///< If the request of the client needs a response.
    QStringList protocolUnknow; ///< List the plugins that doesn't know the protocol of the request.
};

#endif // ENGINESERVER_H
