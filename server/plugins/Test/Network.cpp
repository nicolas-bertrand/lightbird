#include "LightBird.h"
#include "Network.h"

Network::Network(LightBird::IApi &api)
    : ITest(api)
{
}

Network::~Network()
{
}

unsigned int    Network::run()
{
    log.debug("Running the tests of the network...", "Network", "run");
    try
    {
        LightBird::IContexts &c = this->api.contexts();
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
        c1->removePorts(QStringList() << "42" << "43");
        ASSERT(c1->getPorts().isEmpty());
    }
    catch (unsigned int line)
    {
        this->log.debug("Tests of the network failed!", Properties("line", line).toMap(), "Network", "run");
        return (line);
    }
    this->log.debug("Tests of the network successful!", "Network", "run");
    return (0);
}
