#ifndef NETWORK_H
# define NETWORK_H

# include <QObject>

# include "IDoProtocol.h"
# include "IDoDeserializeHeader.h"
# include "IDoDeserializeContent.h"
# include "IDoSerializeContent.h"
# include "ITest.h"

namespace NetworkTest
{
    class ServerEcho;
}

/// @brief Tests the Network of the server.
class Network
    : public QObject
    , public ITest
{
    Q_OBJECT

public:
    Network(LightBird::IApi &api);
    ~Network();

    unsigned int run();

    struct Configuration
    {
        int waitMsec;
        bool displayProgress;
        int displayInterval;
        struct Server
        {
            QString contextName;
            quint16 port; ///< The port of the server test.
            QString protocol;

            struct Synchronous
            {
                bool enable; ///< Enables the test.
                int repeat; ///< The number of times this test is repeated.
                int requestsNumber; ///< The number of synchronous requests to make.
                int dataSizeMin; ///< The minimum size of the random data to send.
                int dataSizeMax; ///< The maximum size of the random data to send.
            };
            Synchronous synchronous;

            struct SingleWriteMultipleRequests
            {
                bool enable; ///< Enables the test.
                int repeat; ///< The number of times this test is repeated.
                int requestsNumber; ///< The number of synchronous requests to make.
                int dataSizeMin; ///< The minimum size of the random data to send.
                int dataSizeMax; ///< The maximum size of the random data to send.
            };
            SingleWriteMultipleRequests singleWriteMultipleRequests;
        };
        Server serverEcho;
    };

private:
    Network(const Network &);
    Network &operator=(const Network &);

    void _testContexts();
    void _testServer();

    Configuration _configuration;
    NetworkTest::ServerEcho *_server;
};

namespace NetworkTest
{
    class ServerEcho
        : public QObject
        , public LightBird::IDoProtocol
        , public LightBird::IDoDeserializeHeader
        , public LightBird::IDoDeserializeContent
        , public LightBird::IDoSerializeContent
    {
        Q_OBJECT
        Q_INTERFACES(LightBird::IDoProtocol
                     LightBird::IDoDeserializeHeader
                     LightBird::IDoDeserializeContent
                     LightBird::IDoSerializeContent)

    public:
        ServerEcho(Network::Configuration::Server &configuration) : _c(configuration) {}

    private:
        bool doProtocol(LightBird::IClient &client, const QByteArray &data, QString &protocol, bool &unknow);
        bool doDeserializeHeader(LightBird::IClient &client, const QByteArray &data, quint64 &used);
        bool doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used);
        bool doSerializeContent(LightBird::IClient &client, QByteArray &data);

        Network::Configuration::Server &_c;
    };
}

#endif // NETWORK_H
