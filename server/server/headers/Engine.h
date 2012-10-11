#ifndef ENGINE_H
# define ENGINE_H

# include <QByteArray>
# include <QObject>
# include <QStringList>

# include "IOnDeserialize.h"
# include "IOnSerialize.h"

# include "Client.h"
# include "Request.h"
# include "Response.h"

/// @brief Interface inherited by all the engines.
class Engine : public QObject
{
    Q_OBJECT

public:
    Engine(Client &client);
    virtual ~Engine();

    /// @brief Run the engine. Process the data stored in this->data, of sent a
    /// new request/response.
    /// @return True while there is enough data to run the engine, and the client
    /// is still connected to the server.
    virtual bool    run() = 0;
    /// @brief Returns the request currently executing by the Engine.
    LightBird::IRequest  &getRequest();
    /// @brief Returns the response currently generated by the Engine.
    LightBird::IResponse &getResponse();
    /// @brief Returns true if the engine has just been cleared and is not running.
    virtual bool    isIdle() = 0;
    /// @brief This method is called each time new data are available for the Engine
    /// and calls LightBird::IOnRead.
    void            onRead();

protected:
    Engine(const Engine &);
    Engine &operator=(const Engine &);

    /// @brief Prepares the Engine to execute an other request.
    virtual void    _clear();
    /// @brief Calls LightBird::IOnWrite.
    void            _onWrite(QByteArray &data);
    /// @brief Calls LightBird::IOnDeserialize.
    void            _onDeserialize(LightBird::IOnDeserialize::Deserialize type);
    /// @brief Calls LightBird::IOnSerialize.
    bool            _onSerialize(LightBird::IOnSerialize::Serialize type);

    Client          &client;  ///< The client for which the engine is running.
    QByteArray      &data;    ///< The raw data received from a client, waiting to be deserialized.
    Request         request;  ///< The request.
    Response        response; ///< The response.
    bool            done;     ///< True if at least one plugin has implemented one of the IDoDeserialize or the IDoSerialize interfaces.
};

#endif // ENGINE_H
