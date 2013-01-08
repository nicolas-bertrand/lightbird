#include "Context.h"
#include "LightBird.h"
#include "Log.h"
#include "Mutex.h"

Context::Context(const QString &id)
    : idPlugin(id)
    , instance(NULL)
    , allProtocols(false)
    , allPorts(false)
    , allModes(true)
    , allTransports(true)
{
}

Context::~Context()
{
}

Context::Context(const Context &context)
{
    *this = context;
}

Context &Context::operator=(const Context &context)
{
    Mutex   mutex(context.mutex, Mutex::READ, "Context", "operator=");

    if (mutex && this != &context)
    {
        this->idPlugin = context.idPlugin;
        this->instance = context.instance;
        this->name = context.name;
        this->allProtocols = context.allProtocols;
        this->allPorts = context.allPorts;
        this->protocols = context.protocols;
        this->ports = context.ports;
        this->methods = context.methods;
        this->types = context.types;
        this->allModes = context.allModes;
        this->allTransports = context.allTransports;
        this->mode = context.mode;
        this->transport = context.transport;
    }
    return (*this);
}

bool    Context::operator==(const Context &context) const
{
    Mutex   m1(this->mutex, Mutex::READ, "Context", "operator==");
    Mutex   m2(context.mutex, Mutex::READ, "Context", "operator==");

    if (this == &context)
        return (true);
    if (m1 && m2 &&
        this->idPlugin == context.idPlugin &&
        this->instance == context.instance &&
        this->name == context.name &&
        this->allProtocols == context.allProtocols &&
        this->allPorts == context.allPorts &&
        this->protocols == context.protocols &&
        this->ports == context.ports &&
        this->methods == context.methods &&
        this->types == context.types &&
        this->allModes == context.allModes &&
        this->allTransports == context.allTransports &&
        this->mode == context.mode &&
        this->transport == context.transport)
        return (true);
    return (false);
}

bool    Context::operator!=(const Context &context) const
{
    return (!(*this == context));
}

bool    Context::operator==(const LightBird::IContext &context) const
{
    return (*this == *((Context *)&context));
}

bool    Context::operator!=(const LightBird::IContext &context) const
{
    return (!(*this == *((Context *)&context)));
}

QString Context::getName() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Context", "getName");

    return (this->name);
}

void    Context::setName(const QString &name)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "setName");

    this->name = name.toLower();
}

QString Context::getMode() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Context", "getMode");

    if (this->allModes)
        return ("");
    else if (this->mode == LightBird::IClient::CLIENT)
        return ("client");
    else if (this->mode == LightBird::IClient::SERVER)
        return ("server");
    return ("");
}

void    Context::setMode(QString mode)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "setMode");

    mode = mode.toLower();
    this->allModes = false;
    if (mode == "client")
        this->mode = LightBird::IClient::CLIENT;
    else if (mode == "server")
        this->mode = LightBird::IClient::SERVER;
    else
        this->allModes = true;
}

QString Context::getTransport() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Context", "getTransport");

    if (this->allTransports)
        return ("");
    else if (this->transport == LightBird::INetwork::TCP)
        return ("TCP");
    else if (this->transport == LightBird::INetwork::UDP)
        return ("UDP");
    return ("");
}

void    Context::setTransport(QString transport)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "setTransport");

    transport = transport.toUpper();
    this->allTransports = false;
    if (transport == "TCP")
        this->transport = LightBird::INetwork::TCP;
    else if (transport == "UDP")
        this->transport = LightBird::INetwork::UDP;
    else
        this->allTransports = true;
}

QStringList Context::getProtocols() const
{
    Mutex       mutex(this->mutex, Mutex::READ, "Context", "getProtocols");
    QStringList protocols(this->protocols);

    if (this->allProtocols)
        protocols.prepend("all");
    return (protocols);
}

void    Context::addProtocols(const QStringList &protocols)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "addProtocols");
    QString protocol;

    QStringListIterator it(protocols);
    while (it.hasNext())
    {
        protocol = it.next().toLower();
        if (protocol == "all")
            this->allProtocols = true;
        else if (!protocol.isEmpty() && !this->protocols.contains(protocol))
            this->protocols.push_back(protocol);
    }
}

void    Context::removeProtocols(const QStringList &protocols)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "removeProtocols");
    QString protocol;

    QStringListIterator it(protocols);
    while (it.hasNext())
    {
        protocol = it.next().toLower();
        if (protocol == "all")
            this->allProtocols = false;
        this->protocols.removeAll(protocol);
    }
}

QStringList Context::getPorts() const
{
    Mutex       mutex(this->mutex, Mutex::READ, "Context", "getPorts");
    QStringList ports;

    if (this->allPorts)
        ports << "all";
    QListIterator<ushort> it(this->ports);
    while (it.hasNext())
        ports << QString::number(it.next());
    return (ports);
}

void    Context::addPorts(const QStringList &ports)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "addPorts");
    QString port;
    ushort  p;

    QStringListIterator it(ports);
    while (it.hasNext())
    {
        port = it.next().toLower();
        p = port.toUShort();
        if (port == "all")
            this->allPorts = true;
        else if (p != 0 && !this->ports.contains(p))
            this->ports.push_back(p);
    }
}

void    Context::addPorts(const QString &ports)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "addPorts");
    ushort  p;

    if (ports.split(' ').contains("all"))
        this->allPorts = true;
    QListIterator<ushort> it(LightBird::parsePorts(ports));
    while (it.hasNext())
    {
        p = it.next();
        if (p != 0 && !this->ports.contains(p))
            this->ports.push_back(p);
    }
}

void    Context::removePorts(const QStringList &ports)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "removePorts");
    QString port;

    QStringListIterator it(ports);
    while (it.hasNext())
    {
        port = it.next().toLower();
        if (port == "all")
            this->allPorts = false;
        this->ports.removeAll(port.toUShort());
    }
}

QStringList Context::getMethods() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Context", "getMethods");

    return (this->methods);
}

void    Context::addMethods(const QStringList &methods)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "addMethods");
    QString method;

    QStringListIterator it(methods);
    while (it.hasNext())
    {
        method = it.next().toLower();
        if (!method.isEmpty() && !this->methods.contains(method))
            this->methods.push_back(method);
    }
}

void    Context::removeMethods(const QStringList &methods)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "removeMethods");

    QStringListIterator it(methods);
    while (it.hasNext())
        this->methods.removeAll(it.next().toLower());
}

QStringList Context::getTypes() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Context", "getTypes");

    return (this->types);
}

void    Context::addTypes(const QStringList &types)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "addTypes");
    QString type;

    QStringListIterator it(types);
    while (it.hasNext())
    {
        type = it.next().toLower();
        if (!type.isEmpty() && !this->types.contains(type))
            this->types.push_back(type);
    }
}

void    Context::removeTypes(const QStringList &types)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "removeTypes");

    QStringListIterator it(types);
    while (it.hasNext())
        this->types.removeAll(it.next().toLower());
}

bool    Context::checkName(QMap<QString, QObject *> &contexts)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Context", "checkName");

    if (this->name.isEmpty())
    {
        this->instance = NULL;
        return (true);
    }
    QMapIterator<QString, QObject *> it(contexts);
    while (it.hasNext())
        if (it.next().key().toLower() == this->name)
        {
            this->instance = it.value();
            return (true);
        }
    LOG_WARNING("Unknow context name", Properties("idPlugin", this->idPlugin).add("unknow name", this->name).add("possible names", QStringList(contexts.keys()).join(' ')));
    return (false);
}

QObject *Context::getInstance() const
{
    return (this->instance);
}

bool    Context::isValid(const Context::Validator &v) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Context", "isValid");

    // Validates the context name
    if (v.names)
    {
        // If we are in the default context (empty name), and the list of names is not empty and does not contains an empty string, the context is not valid
        if (this->name.isEmpty() && !v.names->isEmpty() && !v.names->contains(""))
            return (false);
        // If the name is not in the list and is not empty, the context is not valid
        if (!this->name.isEmpty() && !v.names->contains(this->name, Qt::CaseInsensitive))
            return (false);
    }
    // If there is not all the modes and the mode mismatch, the context is not valid
    if (!this->allModes && this->mode != v.mode)
        return (false);
    // If there is not all the transports  and the transport protocol mismatch, the context is not valid
    if (!this->allTransports && this->transport != v.transport)
        return (false);
    // If the port are different and there is not all ports, the context is not valid
    if (!this->ports.contains(v.port) && !this->allPorts)
    {
        // If the protocol is empty, the context is invalid
        if (v.protocols->isEmpty() || this->protocols.isEmpty())
            return (false);
        // If all the protocols are handled, the context is valid
        if (!this->allProtocols && !v.protocols->contains("all", Qt::CaseInsensitive))
        {
            // Checks if any of the protocols matches
            bool contains = false;
            QStringListIterator it(*v.protocols);
            while (it.hasNext() && !contains)
                if (this->protocols.contains(it.next().toLower()))
                    contains = true;
            // If the protocols doesn't contains one of the given protocols
            if (!contains)
                return (false);
        }
    }
    // Validates the method and the type
    if (v.method && v.type)
    {
        // An empty method means that all the methods are supported
        if (!v.method->isEmpty() && !this->methods.contains(v.method->toLower()) && this->methods.size() > 0)
            return (false);
        // An empty type means that all the types are supported
        if (!v.type->isEmpty() && !this->types.contains(v.type->toLower()) && this->types.size() > 0)
            return (false);
    }
    return (true);
}

QMap<QString, QString>  Context::toMap() const
{
    QMap<QString, QString>  context;
    Mutex   mutex(this->mutex, Mutex::READ, "Context", "toMap");

    if (!this->name.isEmpty())
        context["name"] = this->name;
    if (!this->allModes)
        context["mode"] = (this->mode == LightBird::IClient::SERVER ? "server" : "client");
    else
        context["mode"] = "all";
    if (!this->allTransports)
        context["transport"] = (this->transport == LightBird::INetwork::TCP ? "TCP" : "UDP");
    else
        context["transport"] = "all";
    QStringListIterator itProtocols(this->protocols);
    while (itProtocols.hasNext())
        context.insertMulti("protocol", itProtocols.next());
    QListIterator<ushort> itPorts(this->ports);
    while (itPorts.hasNext())
        context.insertMulti("port", QString::number(itPorts.next()));
    QStringListIterator itMethods(this->methods);
    while (itMethods.hasNext())
        context.insertMulti("method", itMethods.next());
    QStringListIterator itTypes(this->types);
    while (itTypes.hasNext())
        context.insertMulti("type", itTypes.next());
    return (context);
}
