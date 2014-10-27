#ifndef NETWORK_H
# define NETWORK_H

# include <QObject>
# include <QTcpSocket>
# include <QTimer>

# include "IDoProtocol.h"
# include "IDoDeserializeHeader.h"
# include "IDoDeserializeContent.h"
# include "IDoSerializeContent.h"
# include "ITest.h"

namespace Server
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
    void error(unsigned int line);

    struct Configuration
    {

        int waitMsec; ///< The maximum time to wait for an operation to complete.
        bool display; ///< True if the progress of the test should be displayed on the standard output.
        int displayInterval; ///< The interval at which the progress is displayed on the standard output.
        unsigned int seed; ///< The srand seed (0 uses LightBird::srand).

        struct Test
        {
            Test(bool e = true, bool d = true, int r = 1) : enable(e), display(d), repeat(r) {}
            bool enable; ///< Enables the test.
            bool display; ///< True if the progress of the test should be displayed on the standard output.
            int repeat; ///< The number of times this test is repeated.
        };
        struct Range
        {
            Range(double mi = 1, double ma = 100) : min(mi), max(ma) {}
            static inline double random(const Range &range) { return float(qrand()) / float(RAND_MAX) * (range.max - range.min + 1) + range.min; }
            double min;
            double max;
        };

        struct Server
        {
            QString contex;
            quint16 port; ///< The port of the server test.
            QString protocol;

            struct Synchronous : public Test
            {
                int requestsNumber; ///< The number of synchronous requests to make.
                Range dataSize; ///< The size of the random data to send.
            };
            Synchronous synchronous;

            struct SingleWriteMultipleRequests : public Test
            {
                int requestsNumber; ///< The number of synchronous requests to make.
                Range dataSize; ///< The size of the random data to send.
            };
            SingleWriteMultipleRequests singleWriteMultipleRequests;

            struct Asynchronous : public Test
            {
                int connections; ///< The total number of connections.
                Range simultaneousConnections; ///< The number of connections simultaneously connected to the server.
                Range requests; ///< The number of requests per clients.
                Range dataSize; ///< The size of the random data to send.
            };
            Asynchronous asynchronous;
        };
        Server serverEcho;
    };
    Configuration configuration;

private:
    Network(const Network &);
    Network &operator=(const Network &);

    void _testContexts();
    void _testServer();

    Server::ServerEcho *_server;
    unsigned int _errorLine;
};

namespace Server
{
    class AsynchronousConnection;

    class Asynchronous : public QObject
    {
        Q_OBJECT

    public:
        Asynchronous(Network &network);
        inline quint64 duration() const { return _duration; }
        inline quint64 requestsPerSecond() const { return _requestsPerSecond; }
        inline double mbPerSecond() const { return _mbPerSecond; }

    private slots:
        void update();
        void finished(AsynchronousConnection *connection);

    private:
        Network &_n;
        Network::Configuration &_c;
        Network::Configuration::Server &_s;
        Network::Configuration::Server::Asynchronous &_t;
        QList<QSharedPointer<Server::AsynchronousConnection> > _connections;
        QTimer _timer;
        quint64 _requests;
        quint64 _requestsProcessed;
        double _size;
        double _sizeProcessed;
        quint64 _connectionsTotal;
        int _simultaneousConnections;
        quint64 _startTime;

        // Results
        quint64 _duration;
        quint64 _requestsPerSecond;
        double _mbPerSecond;
    };

    class AsynchronousConnection : public QObject
    {
        Q_OBJECT

    public:
        AsynchronousConnection(Network &network);
        inline quint64 requests() { return _requests; }
        inline quint64 requestsTotal() { return _requestsTotal; }
        inline quint64 size() { return _size; }

    signals:
        void finished(AsynchronousConnection *asynchronous);

    private slots:
        void connected();
        void readyRead();
        void timeout();

    private:
        void _write();

        QTcpSocket _socket;
        Network &_n;
        Network::Configuration &_c;
        Network::Configuration::Server &_s;
        Network::Configuration::Server::Asynchronous &_t;
        QTimer _timer;
        unsigned int _line;
        QByteArray _data;
        QByteArray _response;
        quint64 _requests;
        quint64 _requestsTotal;
        quint64 _size;
    };
}

namespace Server
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
