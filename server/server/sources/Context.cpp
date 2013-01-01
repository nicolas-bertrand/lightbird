#include "Context.h"
#include "Log.h"

Context::Context(const QString &id)
    : idPlugin(id)
    , instance(NULL)
    , allProtocols(false)
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
    if (this != &context)
    {
        this->idPlugin = context.idPlugin;
        this->instance = context.instance;
        this->name = context.name;
        this->allProtocols = context.allProtocols;
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

bool    Context::operator==(const Context &context)
{
    if (this == &context)
        return (true);
    if (this->idPlugin == context.idPlugin &&
        this->instance == context.instance &&
        this->name == context.name &&
        this->allProtocols == context.allProtocols &&
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

void    Context::setName(const QString &name)
{
    this->name = name.toLower();
}

void    Context::setMode(QString mode)
{
    mode = mode.toLower();
    this->allModes = false;
    if (mode == "client")
        this->mode = LightBird::IClient::CLIENT;
    else if (mode == "server")
        this->mode = LightBird::IClient::SERVER;
    else
        this->allModes = true;
}

void    Context::setTransport(QString transport)
{
    transport = transport.toUpper();
    this->allTransports = false;
    if (transport == "TCP")
        this->transport = LightBird::INetwork::TCP;
    else if (transport == "UDP")
        this->transport = LightBird::INetwork::UDP;
    else
        this->allTransports = true;
}

void    Context::addProtocols(const QStringList &protocols)
{
    QString protocol;

    QStringListIterator it(protocols);
    while (it.hasNext())
    {
        protocol = it.next().toLower();
        if (protocol == "all")
            this->allProtocols = true;
        if (!protocol.isEmpty() && !this->protocols.contains(protocol))
            this->protocols.push_back(protocol);
    }
}

void    Context::addPorts(const QStringList &ports)
{
    QString port;
    ushort  p;

    QStringListIterator it(ports);
    while (it.hasNext())
    {
        port = it.next().toLower();
        p = port.toUShort();
        if (port != "all" && p == 0)
            continue ;
        if (!this->ports.contains(p))
            this->ports.push_back(p);
    }
}

void    Context::addMethods(const QStringList &methods)
{
    QString method;

    QStringListIterator it(methods);
    while (it.hasNext())
    {
        method = it.next().toLower();
        if (!method.isEmpty() && !this->methods.contains(method))
            this->methods.push_back(method);
    }
}

void    Context::addTypes(const QStringList &types)
{
    QString type;

    QStringListIterator it(types);
    while (it.hasNext())
    {
        type = it.next().toLower();
        if (!type.isEmpty() && !this->types.contains(type))
            this->types.push_back(type);
    }
}

bool    Context::checkName(QMap<QString, QObject *> &contexts)
{
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
    // If the port are different and it doesn't contains 0, the context is not valid (because 0 means that all the ports are supported)
    if (!this->ports.contains(v.port) && !this->ports.contains(0))
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
