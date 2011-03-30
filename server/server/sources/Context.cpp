#include "Context.h"

Context::Context(QObject *parent) : QObject(parent)
{
}

Context::~Context()
{
}

Context::Context(const Context &context) : QObject()
{
    *this = context;
}

Context &Context::operator=(const Context &context)
{
    if (this != &context)
    {
        this->transport = context.transport;
        this->protocols = context.protocols;
        this->ports = context.ports;
        this->methods = context.methods;
        this->types = context.types;
    }
    return (*this);
}

bool    Context::operator==(const Context &context)
{
    if (this == &context)
        return (true);
    if (this->transport == context.transport &&
        this->protocols == context.protocols &&
        this->ports == context.ports &&
        this->methods == context.methods &&
        this->types == context.types)
        return (true);
    return (false);
}

void    Context::setTransport(const QString &transport)
{
    this->transport = transport;
}

void    Context::setProtocol(const QString &protocol)
{
    if (!this->protocols.contains(protocol))
        this->protocols.push_back(protocol);
}

void    Context::setPort(unsigned short port)
{
    if (!this->ports.contains(port))
        this->ports.push_back(port);
}

void    Context::setMethod(const QString &method)
{
    if (!this->methods.contains(method))
        this->methods.push_back(method);
}

void    Context::setType(const QString &type)
{
    if (!this->types.contains(type))
        this->types.push_back(type);
}

bool    Context::isValid(const QString &transport, const QStringList &protocols, unsigned short port) const
{
    // If the transport protocol mismatch and is not empty, the context is not valid (because empty means that all the transports are supported)
    if (this->transport != transport && !this->transport.isEmpty())
        return (false);
    // If the port are different and it doesn't contains 0, the context is not valid (because 0 means that all the ports are supported)
    if (!this->ports.contains(port) && !this->ports.contains(0))
    {
        // If the protocol is empty, the context is invalid
        if (protocols.isEmpty() || this->protocols.isEmpty())
            return (false);
        // If all the protocols are handled, the context is valid
        if (this->protocols.contains("all", Qt::CaseInsensitive) || protocols.contains("all", Qt::CaseInsensitive))
            return (true);
        // Checks if any of the protocols matches
        bool contains = false;
        QStringListIterator it(protocols);
        while (it.hasNext() && !contains)
            if (this->protocols.contains(it.next()))
                contains = true;
        // If the protocols doesn't contains one of the given protocols
        if (!contains)
            return (false);
    }
    return (true);
}

bool    Context::isValid(const QString &transport, const QStringList &protocols, unsigned short port, const QString &method, const QString &type) const
{
    // Check if the transport, the port, and the protocol are valid
    if (!this->isValid(transport, protocols, port))
        return (false);
    // An empty method means that all the methods are supported
    if (!method.isEmpty() && !this->methods.contains(method.toLower()) && this->methods.size() > 0)
        return (false);
    // An empty type means that all the types are supported
    if (!type.isEmpty() && !this->types.contains(type.toLower()) && this->types.size() > 0)
        return (false);
    return (true);
}

QMap<QString, QString>  Context::toMap() const
{
    QMap<QString, QString>  context;

    if (!this->transport.isEmpty())
        context["transport"] = this->transport;
    QStringListIterator itProtocols(this->protocols);
    while (itProtocols.hasNext())
        context.insertMulti("protocol", itProtocols.next());
    QListIterator<unsigned short> itPorts(this->ports);
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
