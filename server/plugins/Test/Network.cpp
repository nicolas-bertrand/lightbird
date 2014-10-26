#include <QTcpSocket>
#include "LightBird.h"
#include "Network.h"

Network::Network(LightBird::IApi &api)
    : ITest(api)
    , _server(NULL)
{
    _configuration.waitMsec = 5000;
    _configuration.displayProgress = true;
    _configuration.displayInterval = 500;
    _configuration.serverEcho.contextName = "serverEcho";
    _configuration.serverEcho.port = _api.configuration(true).get("network/serverPort", "9090").toUShort();
    _configuration.serverEcho.protocol = "test_server_echo";

    _configuration.serverEcho.synchronous.enable = true;
    _configuration.serverEcho.synchronous.repeat = 1;
    _configuration.serverEcho.synchronous.requestsNumber = 10000;
    _configuration.serverEcho.synchronous.dataSizeMin = 1;
    _configuration.serverEcho.synchronous.dataSizeMax = 10000;

    _configuration.serverEcho.singleWriteMultipleRequests.enable = true;
    _configuration.serverEcho.singleWriteMultipleRequests.repeat = 10;
    _configuration.serverEcho.singleWriteMultipleRequests.requestsNumber = 400;
    _configuration.serverEcho.singleWriteMultipleRequests.dataSizeMin = 1;
    _configuration.serverEcho.singleWriteMultipleRequests.dataSizeMax = 1000;
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
    Configuration::Server &s = _configuration.serverEcho;
    Configuration &c = _configuration;

    _log.info("Running unit tests of the server...", "Network", "_testServer");
    try
    {
        // Creates the server
        _server = new NetworkTest::ServerEcho(s);
        ASSERT(_api.network().openPort(s.port, QStringList(s.protocol), LightBird::INetwork::TCP, QStringList() << s.contextName));
        ASSERT(_api.contexts().declareInstance(s.contextName, _server));
        LightBird::IContext *context = _api.contexts().add(s.contextName);
        ASSERT(context != NULL);
        context->addPort(s.port);
        context->addProtocol(s.protocol);
        ::qsrand((unsigned int)(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000));

        // Synchronous requests
        if (s.synchronous.enable)
        {
            Configuration::Server::Synchronous &t = _configuration.serverEcho.synchronous;
            _log.info("Performing the synchronous requests test...", Properties("requestsNumber", t.requestsNumber).add("repeat", t.repeat).add("dataSizeMin", t.dataSizeMin).add("dataSizeMax", t.dataSizeMax).toMap(), "Network", "_testServer");
            QTcpSocket tcpSocket;
            tcpSocket.connectToHost(QHostAddress::LocalHostIPv6, s.port);
            ASSERT(tcpSocket.waitForConnected(c.waitMsec));
            for (int repeat = 0; repeat < s.synchronous.repeat; ++repeat)
            {
                qint64 time = QDateTime::currentMSecsSinceEpoch();
                int requestsProcessed = 0;
                for (int i = 0; i < t.requestsNumber; ++i)
                {
                    QByteArray data;
                    data.resize(float(qrand()) / float(RAND_MAX) * (t.dataSizeMax - t.dataSizeMin) + t.dataSizeMin);
                    for (int j = 0; j < data.size(); ++j)
                        data[j] = qrand() % 0xFF;
                    QByteArray request(QByteArray::number(data.size()) + "\necho\n" + data);
                    ASSERT(tcpSocket.write(request.data(), request.size()) && tcpSocket.waitForBytesWritten(c.waitMsec));
                    QByteArray response;
                    while (response != data)
                        ASSERT(tcpSocket.waitForReadyRead(c.waitMsec) && !(response += tcpSocket.readAll()).isEmpty());

                    // Displays the progress
                    if (c.displayProgress && QDateTime::currentMSecsSinceEpoch() - time > c.displayInterval)
                    {
                        time = QDateTime::currentMSecsSinceEpoch();
                        QString percent = QString::number(qMin(i / float(t.requestsNumber) * 100.0, 100.0), 'f', 0);
                        std::cout << "[" << QString(2 - percent.size(), ' ').toStdString() << percent.toStdString() << "%] " << i << "/" << t.requestsNumber << " - " << ((i - requestsProcessed) / float(c.displayInterval / 1000.0)) << " per sec" << std::endl;
                        requestsProcessed = i;
                    }
                }
            }
        }

        // Single write multiple requests
        if (s.singleWriteMultipleRequests.enable)
        {
            Configuration::Server::SingleWriteMultipleRequests &t = _configuration.serverEcho.singleWriteMultipleRequests;
            _log.info("Performing the single write multiple requests test...", Properties("requestsNumber", t.requestsNumber).add("repeat", t.repeat).add("dataSizeMin", t.dataSizeMin).add("dataSizeMax", t.dataSizeMax).toMap(), "Network", "_testServer");
            QTcpSocket tcpSocket;
            tcpSocket.connectToHost(QHostAddress::LocalHostIPv6, s.port);
            ASSERT(tcpSocket.waitForConnected(c.waitMsec));
            for (int repeat = 0; repeat < s.singleWriteMultipleRequests.repeat; ++repeat)
            {
                QByteArray requests;
                std::cout << "Generating the requests..." << std::endl;
                for (int i = 0; i < t.requestsNumber; ++i)
                {
                    QByteArray data;
                    data.resize(float(qrand()) / float(RAND_MAX) * (t.dataSizeMax - t.dataSizeMin) + t.dataSizeMin);
                    for (int j = 0; j < data.size(); ++j)
                        data[j] = qrand() % 0xFF;
                    requests += (QByteArray::number(data.size()) + "\necho\n" + data);
                }

                std::cout << "Sending a batch of " << t.requestsNumber << " requests (" << (requests.size() / (1024.0 * 1024.0)) << " Mb)..." << std::endl;
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
                        ASSERT(tcpSocket.waitForReadyRead(c.waitMsec));
                        QByteArray read = tcpSocket.read(data.size() - response.size());
                        ASSERT(!read.isEmpty());
                        response += read;
                        if (response.size() >= data.size())
                            ASSERT(response == data);
                    }

                    // Displays the progress
                    if (c.displayProgress && QDateTime::currentMSecsSinceEpoch() - time > c.displayInterval)
                    {
                        time = QDateTime::currentMSecsSinceEpoch();
                        QString percent = QString::number(qMin(i / float(t.requestsNumber) * 100.0, 100.0), 'f', 0);
                        std::cout << "[" << QString(2 - percent.size(), ' ').toStdString() << percent.toStdString() << "%] " << i << "/" << t.requestsNumber << " - " << ((i - requestsProcessed) / float(c.displayInterval / 1000.0)) << " per sec" << std::endl;
                        requestsProcessed = i;
                    }
                }
            }
        }
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the server failed!", Properties("line", line).toMap(), "Network", "_testServer");
        throw line;
    }
    _log.trace("Unit tests of the contexts successful!", "Network", "_testServer");
}

namespace NetworkTest
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

} // NetworkTest















































