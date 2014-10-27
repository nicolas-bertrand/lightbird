#include <QTcpSocket>
#include <QThread>
#include "LightBird.h"
#include "Plugin.h"
#include "Network.h"

Network::Network(LightBird::IApi &api)
    : ITest(api)
    , _server(NULL)
    , _errorLine(0)
{
    configuration.waitMsec = 5000;
    configuration.display = true;
    configuration.displayInterval = 500;
    configuration.seed = 0;
    configuration.serverEcho.contex = "serverEcho";
    configuration.serverEcho.port = _api.configuration(true).get("network/serverPort", "9090").toUShort();
    configuration.serverEcho.protocol = "test_server_echo";

    configuration.serverEcho.synchronous.enable = true;
    configuration.serverEcho.synchronous.display = true;
    configuration.serverEcho.synchronous.repeat = 1;
    configuration.serverEcho.synchronous.requestsNumber = 5000;
    configuration.serverEcho.synchronous.dataSize = Configuration::Range(1, 10000);

    configuration.serverEcho.singleWriteMultipleRequests.enable = true;
    configuration.serverEcho.singleWriteMultipleRequests.display = true;
    configuration.serverEcho.singleWriteMultipleRequests.repeat = 1;
    configuration.serverEcho.singleWriteMultipleRequests.requestsNumber = 2000;
    configuration.serverEcho.singleWriteMultipleRequests.dataSize = Configuration::Range(1, 10000);

    configuration.serverEcho.asynchronous.enable = true;
    configuration.serverEcho.asynchronous.display = true;
    configuration.serverEcho.asynchronous.repeat = 1;
    configuration.serverEcho.asynchronous.connections = 1000;
    configuration.serverEcho.asynchronous.simultaneousConnections = Configuration::Range(1, qMin(300, configuration.serverEcho.asynchronous.connections));
    configuration.serverEcho.asynchronous.requests = Configuration::Range(1, 50);
    configuration.serverEcho.asynchronous.dataSize = Configuration::Range(1, 10000);
}

Network::~Network()
{
    delete _server;
}

unsigned int Network::run()
{
    _log.debug("Running the unit tests of the network...", "Network", "run");
    try
    {
        _testContexts();
        _testServer();
    }
    catch (unsigned int line)
    {
        _log.debug("Unit tests of the network failed!", Properties("line", line).toMap(), "Network", "run");
        return (line);
    }
    _log.debug("Unit tests of the network successful!", "Network", "run");
    return (0);
}

void Network::error(unsigned int line)
{
    if (!_errorLine)
        _errorLine = line;
    QThread::currentThread()->quit();
}

void Network::_testContexts()
{
    _log.trace("Running unit tests of the contexts...", "Network", "_testContexts");
    try
    {
        LightBird::IContexts &c = _api.contexts();
        LightBird::IContext *c1, *c2, *c3, *c4;

        ASSERT(c.declareInstance("test1", this));
        ASSERT(!c.declareInstance("test1", this));
        ASSERT(c.get().isEmpty());
        ASSERT(c1 = c.add("test1"));
        ASSERT(!c.add("test2"));
        ASSERT(c2 = c.add("test1"));
        ASSERT(*c1 == *c2);
        ASSERT(!(*c1 != *c2));
        ASSERT(c.get().values("test1").size() == 2);
        ASSERT(c.get().value("test1") == c2);
        ASSERT(c.get().values("test1").last() == c1);
        ASSERT(c.get(QStringList("test2")).isEmpty());
        ASSERT(c.get(QStringList("")).isEmpty());
        ASSERT(c.get(QStringList("test1")).size() == 2);
        ASSERT(c3 = c.add(""));
        ASSERT(*c3 != *c2);
        ASSERT(!(*c3 == *c1));
        ASSERT(c.get(QStringList("")).size() == 1);
        ASSERT(c.get(QStringList("")).value("") == c3);
        ASSERT(c.get().size() == 3);
        ASSERT(c.get().value("") == c3);
        ASSERT(!c.clone(c3, "test2"));
        ASSERT(c4 = c.clone(c3, ""));
        ASSERT(*c3 == *c4);
        ASSERT(c.get(QStringList("")).size() == 2);
        c.remove(c4);
        ASSERT(c.get(QStringList("")).size() == 1);

        ASSERT(c1->getName() == "test1");
        ASSERT(c3->getName().isEmpty());

        ASSERT(c1->getMode() == "");
        c1->setMode("server");
        ASSERT(c1->getMode() == "server");
        c1->setMode("client");
        ASSERT(c1->getMode() == "client");
        c1->setMode("other");
        ASSERT(c1->getMode() == "");
        c1->setMode("SERVER");
        ASSERT(c1->getMode() == "server");

        ASSERT(c1->getTransport() == "");
        c1->setTransport("TCP");
        ASSERT(c1->getTransport() == "TCP");
        c1->setTransport("Udp");
        ASSERT(c1->getTransport() == "UDP");
        c1->setTransport("other");
        ASSERT(c1->getTransport() == "");

        ASSERT(c1->getMethods().isEmpty());
        c1->addMethods(QStringList() << "a" << "a" << "b" << "c" << "");
        ASSERT(c1->getMethods() == QStringList() << "a" << "b" << "c");
        c1->removeMethods(QStringList() << "c");
        ASSERT(c1->getMethods() == QStringList() << "a" << "b");
        c1->removeMethods(QStringList() << "c");
        ASSERT(c1->getMethods() == QStringList() << "a" << "b");
        c1->removeMethods(QStringList() << "a" << "b");
        ASSERT(c1->getMethods().isEmpty());

        ASSERT(c1->getTypes().isEmpty());
        c1->addTypes(QStringList() << "a" << "a" << "b" << "c" << "");
        ASSERT(c1->getTypes() == QStringList() << "a" << "b" << "c");
        c1->removeTypes(QStringList() << "c");
        c1->addTypes(QStringList());
        ASSERT(c1->getTypes() == QStringList() << "a" << "b");
        c1->removeTypes(QStringList() << "c");
        c1->addTypes(QStringList("a"));
        ASSERT(c1->getTypes() == QStringList() << "a" << "b");
        c1->removeTypes(QStringList() << "a" << "b");
        ASSERT(c1->getTypes().isEmpty());

        ASSERT(c1->getProtocols().isEmpty());
        c1->addProtocols(QStringList() << "a" << "a" << "b" << "ALl" << "c" << "");
        ASSERT(c1->getProtocols() == QStringList() << "all" << "a" << "b" << "c");
        c1->removeProtocols(QStringList() << "all");
        ASSERT(c1->getProtocols() == QStringList() << "a" << "b" << "c");
        c1->removeProtocols(QStringList() << "c");
        ASSERT(c1->getProtocols() == QStringList() << "a" << "b");
        c1->removeProtocols(QStringList() << "c");
        ASSERT(c1->getProtocols() == QStringList() << "a" << "b");
        c1->removeProtocols(QStringList() << "a" << "b");
        ASSERT(c1->getProtocols().isEmpty());

        ASSERT(c1->getPorts().isEmpty());
        c1->addPorts(QStringList() << "42" << "42" << "43" << "All" << "44" << "a" << "");
        ASSERT(c1->getPorts() == QStringList() << "all" << "42" << "43" << "44");
        c1->removePorts(QStringList() << "alL");
        ASSERT(c1->getPorts() == QStringList() << "42" << "43" << "44");
        c1->removePorts(QStringList() << "44");
        ASSERT(c1->getPorts() == QStringList() << "42" << "43");
        c1->removePorts(QStringList() << "44");
        ASSERT(c1->getPorts() == QStringList() << "42" << "43");
        c1->addPort(15);
        ASSERT(c1->getPorts() == QStringList() << "42" << "43" << "15");
        c1->addPort(15);
        ASSERT(c1->getPorts() == QStringList() << "42" << "43" << "15");
        c1->addPort(0);
        ASSERT(c1->getPorts() == QStringList() << "42" << "43" << "15");
        c1->removePort(42);
        ASSERT(c1->getPorts() == QStringList() << "43" << "15");
        c1->removePort(43);
        ASSERT(c1->getPorts() == QStringList() << "15");
        c1->removePorts(QStringList() << "15");
        ASSERT(c1->getPorts().isEmpty());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the contexts failed!", Properties("line", line).toMap(), "Network", "");
        throw line;
    }
    _log.trace("Unit tests of the contexts successful!", "Network", "_testContexts");
}

void Network::_testServer()
{
    Configuration::Server &s = configuration.serverEcho;
    Configuration &c = configuration;

    _log.info("Running unit tests of the server...", "Network", "_testServer");
    try
    {
        // Creates the server
        _server = new Server::ServerEcho(s);
        ASSERT(_api.network().openPort(s.port, QStringList(s.protocol), LightBird::INetwork::TCP, QStringList() << s.contex));
        ASSERT(_api.contexts().declareInstance(s.contex, _server));
        LightBird::IContext *context = _api.contexts().add(s.contex);
        ASSERT(context != NULL);
        context->addPort(s.port);
        context->addProtocol(s.protocol);
        if (!configuration.seed)
            LightBird::srand();
        else
            ::qsrand(configuration.seed);

        // Synchronous requests
        if (s.synchronous.enable)
        {
            Configuration::Server::Synchronous &t = configuration.serverEcho.synchronous;
            qint64 startTime = QDateTime::currentMSecsSinceEpoch();
            _log.info("Performing the synchronous requests test...", Properties("requestsNumber", t.requestsNumber).add("repeat", t.repeat).add("dataSize.min", t.dataSize.min).add("dataSize.max", t.dataSize.max).toMap(), "Network", "_testServer");
            QTcpSocket tcpSocket;
            tcpSocket.connectToHost(QHostAddress::LocalHostIPv6, s.port);
            ASSERT(tcpSocket.waitForConnected(c.waitMsec));
            for (int repeat = 0; repeat < t.repeat; ++repeat)
            {
                qint64 time = QDateTime::currentMSecsSinceEpoch();
                int requestsProcessed = 0;
                for (int i = 0; i < t.requestsNumber; ++i)
                {
                    QByteArray data;
                    data.resize(Network::Configuration::Range::random(t.dataSize));
                    for (int j = 0; j < data.size(); ++j)
                        data[j] = qrand() % 0xFF;
                    QByteArray request(QByteArray::number(data.size()) + "\necho\n" + data);
                    ASSERT(tcpSocket.write(request.data(), request.size()) && tcpSocket.waitForBytesWritten(c.waitMsec));
                    QByteArray response;
                    while (response != data)
                        ASSERT(tcpSocket.waitForReadyRead(c.waitMsec) && !(response += tcpSocket.readAll()).isEmpty());

                    // Displays the progress
                    if (c.display && t.display && QDateTime::currentMSecsSinceEpoch() - time > c.displayInterval)
                    {
                        time = QDateTime::currentMSecsSinceEpoch();
                        QString percent = QString::number(qMin(i / double(t.requestsNumber) * 100.0, 100.0), 'f', 0);
                        std::cout << "[" << QString(2 - percent.size(), ' ').toStdString() << percent.toStdString() << "%] " << i << "/" << t.requestsNumber << " - " << ((i - requestsProcessed) / double(c.displayInterval / 1000.0)) << " per sec" << std::endl;
                        requestsProcessed = i;
                    }
                }
                double duration = (QDateTime::currentMSecsSinceEpoch() - startTime) / 1000.0;
                _log.info("Synchronous test successful!", Properties("duration", duration).add("requests/s", quint64(t.requestsNumber / duration)).toMap(), "Network", "_testServer");
            }
        }

        // Single write multiple requests
        if (s.singleWriteMultipleRequests.enable)
        {
            Configuration::Server::SingleWriteMultipleRequests &t = configuration.serverEcho.singleWriteMultipleRequests;
            _log.info("Performing the single write multiple requests test...", Properties("requestsNumber", t.requestsNumber).add("repeat", t.repeat).add("dataSize.min", t.dataSize.min).add("dataSize.max", t.dataSize.max).toMap(), "Network", "_testServer");
            QTcpSocket tcpSocket;
            tcpSocket.connectToHost(QHostAddress::LocalHostIPv6, s.port);
            ASSERT(tcpSocket.waitForConnected(c.waitMsec));
            for (int repeat = 0; repeat < t.repeat; ++repeat)
            {
                QByteArray requests;
                qint64 startTime = QDateTime::currentMSecsSinceEpoch();
                if (c.display && t.display)
                    std::cout << "Generating the requests..." << std::endl;
                for (int i = 0; i < t.requestsNumber; ++i)
                {
                    QByteArray data;
                    data.resize(Network::Configuration::Range::random(t.dataSize));
                    for (int j = 0; j < data.size(); ++j)
                        data[j] = qrand() % 0xFF;
                    requests += (QByteArray::number(data.size()) + "\necho\n" + data);
                }

                if (c.display && t.display)
                    std::cout << "Sending a batch of " << t.requestsNumber << " requests (" << (requests.size() / (1024.0 * 1024.0)) << " MB)..." << std::endl;
                ASSERT(tcpSocket.write(requests.data(), requests.size()) == requests.size());
                ASSERT(tcpSocket.waitForBytesWritten(c.waitMsec));

                // Checks the responses
                int p = 0;
                int commandSize = QString("\necho\n").size();
                qint64 time = QDateTime::currentMSecsSinceEpoch();
                int requestsProcessed = 0;
                for (int i = 0; i < t.requestsNumber; ++i)
                {
                    int index = requests.indexOf('\n', p);
                    int size = requests.mid(p, requests.indexOf('\n', p) - p).toInt();
                    p = index + commandSize + size;
                    QByteArray data = requests.mid(index + commandSize, size);
                    QByteArray response;
                    while (data.size() > response.size())
                    {
                        if (tcpSocket.size() <= 0)
                            ASSERT(tcpSocket.waitForReadyRead(c.waitMsec));
                        QByteArray read = tcpSocket.read(data.size() - response.size());
                        ASSERT(!read.isEmpty());
                        response += read;
                        if (response.size() >= data.size())
                            ASSERT(response == data);
                    }

                    // Displays the progress
                    if (c.display && t.display && QDateTime::currentMSecsSinceEpoch() - time > c.displayInterval)
                    {
                        time = QDateTime::currentMSecsSinceEpoch();
                        QString percent = QString::number(qMin(i / double(t.requestsNumber) * 100.0, 100.0), 'f', 0);
                        std::cout << "[" << QString(2 - percent.size(), ' ').toStdString() << percent.toStdString() << "%] " << i << "/" << t.requestsNumber << " - " << ((i - requestsProcessed) / double(c.displayInterval / 1000.0)) << " per sec" << std::endl;
                        requestsProcessed = i;
                    }
                }
                double duration = (QDateTime::currentMSecsSinceEpoch() - startTime) / 1000.0;
                _log.info("Single write multiple requests test successful!", Properties("duration", duration).add("requests/s", quint64(t.requestsNumber / duration)).toMap(), "Network", "_testServer");
            }
        }

        // Asynchronous requests
        if (s.asynchronous.enable)
        {
            Configuration::Server::Asynchronous &t = configuration.serverEcho.asynchronous;
            _log.info("Performing the asynchronous requests test...", Properties("clients", t.connections).add("simultaneousClients.min", t.simultaneousConnections.min).add("simultaneousClients.max", t.simultaneousConnections.max).add("requests.min", t.requests.min).add("requests.max", t.requests.max).add("dataSize.min", t.dataSize.min).add("dataSize.max", t.dataSize.max).toMap(), "Network", "_testServer");
            for (int repeat = 0; repeat < t.repeat; ++repeat)
            {
                Server::Asynchronous asynchronous(*this);
                Plugin::execute();
                if (_errorLine)
                    throw (_errorLine);
                _log.info("Asynchronous test successful!", Properties("duration", asynchronous.duration() / 1000.0).add("requests/s", asynchronous.requestsPerSecond()).add("MB/s", uint(asynchronous.mbPerSecond() * 10) / 10.0).toMap(), "Network", "_testServer");
            }
        }
    }
    catch (unsigned int line)
    {
        _log.trace("Tests of the server failed!", Properties("line", line).toMap(), "Network", "_testServer");
        throw line;
    }
    _log.trace("Tests of the server successful!", "Network", "_testServer");
}

#undef ASSERT
#define ASSERT(a)\
{\
if (!(a))\
    _n.error(((unsigned int)__LINE__));\
else (void)0;\
}\
(void)0\

#define TIMER _timer.start(_c.waitMsec); _line = __LINE__

// Asynchronous
namespace Server
{
    Asynchronous::Asynchronous(Network &n)
        : _n(n)
        , _c(n.configuration)
        , _s(n.configuration.serverEcho)
        , _t(n.configuration.serverEcho.asynchronous)
        , _requests(0)
        , _requestsProcessed(0)
        , _size(0)
        , _sizeProcessed(0)
        , _connectionsTotal(0)
    {
        _startTime = QDateTime::currentMSecsSinceEpoch();
        _simultaneousConnections = Network::Configuration::Range::random(_t.simultaneousConnections);
        for (int i = 0; i < _simultaneousConnections; ++i)
        {
            _connections.append(QSharedPointer<Server::AsynchronousConnection>(new Server::AsynchronousConnection(_n)));
            QObject::connect(_connections.last().data(), SIGNAL(finished(AsynchronousConnection*)), this, SLOT(finished(AsynchronousConnection*)), Qt::QueuedConnection);
            _connectionsTotal++;
        }
        if (_c.display && _t.display)
        {
            QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
            _timer.start(_c.displayInterval);
        }
    }

    void Asynchronous::update()
    {
        quint64 requests = 0;
        quint64 requestsTotal = 0;
        double size = 0;

        for (int i = 0; i < _connections.size(); ++i)
        {
            requests += _connections[i]->requests();
            requestsTotal += _connections[i]->requestsTotal();
            size += _connections[i]->size();
        }

        double pctFinished = (_connectionsTotal - _connections.size()) / double(_t.connections);
        double pctOpened = (_connections.size()) / double(_t.connections) * (requests / double(requestsTotal));
        QString percent = QString::number(qMin((pctFinished + pctOpened) * 100.0, 100.0), 'f', 0);
        std::cout << "[" << QString(2 - percent.size(), ' ').toStdString() << percent.toStdString() << "%] ";

        requests += _requestsProcessed;
        quint64 requestsPerSec = ((requests - _requests) / double(_c.displayInterval / 1000.0));
        size = quint64(((size + _sizeProcessed) / (1024 * 1024)) * 1000) / 1000.0;
        quint64 sizePerSec = ((size - _size) / double(_c.displayInterval / 1000.0));
        std::cout << requests << " (" << requestsPerSec << "/s) requests, " << size << " (" << sizePerSec << "/s) MB, " << _connections.size() << "/" << (_connectionsTotal - _connections.size()) << "/" << _t.connections << " connections" << std::endl;

        _requests = requests;
        _size = size;
    }

    void Asynchronous::finished(AsynchronousConnection *connection)
    {
        for (int i = 0; i < _connections.size(); ++i)
            if (_connections[i].data() == connection)
            {
                if (_connections.size() == _simultaneousConnections)
                    _simultaneousConnections = Network::Configuration::Range::random(_t.simultaneousConnections);
                _requestsProcessed += _connections[i]->requests();
                _sizeProcessed += _connections[i]->size();
                _connections.removeAt(i);
                break;
            }
        for (int i = 0, s = rand() % 5; (_connections.size() == 0 || (_connections.size() < _simultaneousConnections && i < s)) && (int)_connectionsTotal < _t.connections; ++i)
        {
            _connections.append(QSharedPointer<Server::AsynchronousConnection>(new Server::AsynchronousConnection(_n)));
            QObject::connect(_connections.last().data(), SIGNAL(finished(AsynchronousConnection*)), this, SLOT(finished(AsynchronousConnection*)), Qt::QueuedConnection);
            _connectionsTotal++;
        }
        if (_connections.isEmpty())
        {
            _duration = QDateTime::currentMSecsSinceEpoch() - _startTime;
            _requestsPerSecond = _requests / (_duration / 1000.0);
            _mbPerSecond = (_sizeProcessed / (1024 * 1024)) / (_duration / 1000.0);
            QThread::currentThread()->quit();
        }
    }

    AsynchronousConnection::AsynchronousConnection(Network &n)
        : _n(n)
        , _c(n.configuration)
        , _s(n.configuration.serverEcho)
        , _t(n.configuration.serverEcho.asynchronous)
        , _line(0)
        , _requests(0)
        , _size(0)
    {
        _requestsTotal = Network::Configuration::Range::random(_t.requests);
        QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(timeout()));
        _timer.setSingleShot(true);
        QObject::connect(&_socket, SIGNAL(connected()), this, SLOT(connected()));
        QObject::connect(&_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        QObject::disconnect();
        _socket.connectToHost(QHostAddress::LocalHostIPv6, _s.port);
        TIMER;
    }

    void AsynchronousConnection::connected()
    {
        _timer.stop();

        _write();
    }

    void AsynchronousConnection::readyRead()
    {
        _timer.stop();

        int oldSize = _response.size();
        _response += _socket.readAll();
        ASSERT(oldSize != _response.size());
        ASSERT(_response.size() <= _data.size());
        if (_response.size() == _data.size())
        {
            ASSERT(_response == _data);
            _write();
        }
        else
            TIMER;
    }

    void AsynchronousConnection::_write()
    {
        _response.clear();
        _data.clear();
        if (_requests >= _requestsTotal)
        {
            emit finished(this);
            return ;
        }
        _data.resize(Network::Configuration::Range::random(_t.dataSize));
        for (int j = 0; j < _data.size(); ++j)
            _data[j] = qrand() % 0xFF;
        QByteArray request(QByteArray::number(_data.size()) + "\necho\n" + _data);
        ASSERT(_socket.write(request.data(), request.size()) == request.size());
        _size += request.size();
        _requests++;
        TIMER;
    }

    void AsynchronousConnection::timeout()
    {
        _n.error(_line);
    }
}

// ServerEcho
namespace Server
{
    bool ServerEcho::doProtocol(LightBird::IClient &client, const QByteArray &, QString &protocol, bool &)
    {
        protocol = _c.protocol;
        client.getRequest().getContent().setStorage(LightBird::IContent::BYTEARRAY);
        return true;
    }

    bool ServerEcho::doDeserializeHeader(LightBird::IClient &client, const QByteArray &data, quint64 &used)
    {
        if (client.getRequest().getContent().size() > 0)
        {
            int c = client.getRequest().getContent().getData().count('\n');
            if (c + data.count('\n') >= 2)
            {
                QByteArray d = client.getRequest().getContent().getData() + data.left(100);
                int i = d.indexOf('\n');
                int j = d.indexOf('\n', i + 1);

                client.getRequest().getInformations().insert("size", d.left(i).toInt());
                client.getRequest().getInformations().insert("command", d.mid(i + 1, j - i - 1));
                used = j + 1 - client.getRequest().getContent().size();
                client.getRequest().getContent().setStorage(LightBird::IContent::BYTEARRAY);
                return true;
            }
        }
        if (data.count('\n') >= 2)
        {
            int i = data.indexOf('\n');
            int j = data.indexOf('\n', i + 1);

            client.getRequest().getInformations().insert("size", data.left(i).toInt());
            client.getRequest().getInformations().insert("command", data.mid(i + 1, j - i - 1));
            used = j + 1;
            return true;
        }
        if (data.size() > 10 || client.getRequest().getContent().size() > 10)
        {
            used = data.size();
            client.disconnect(true);
            return true;
        }
        client.getRequest().getContent().setData(data);
        return false;
    }

    bool ServerEcho::doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used)
    {
        LightBird::IContent &content = client.getRequest().getContent();
        int contentSize = client.getRequest().getInformations()["size"].toInt();
        QString command = client.getRequest().getInformations()["command"].toString();

        if (data.size() + content.size() >= contentSize)
        {
            if (command == "echo")
            {
                used = contentSize - content.size();
                client.getRequest().getContent().setData(data.left(contentSize - content.size()));
            }
            return true;
        }
        client.getRequest().getContent().setData(data);
        used = data.size();
        return false;
    }

    bool ServerEcho::doSerializeContent(LightBird::IClient &client, QByteArray &data)
    {
        data = client.getRequest().getContent().getData();
        return true;
    }

} // ServerEcho















































