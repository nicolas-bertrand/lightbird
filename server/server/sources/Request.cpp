#include "Request.h"

Request::Request(QObject *parent) : QObject(parent)
{
    this->clear();
}

Request::~Request()
{
}

const QString           &Request::getProtocol() const
{
    return (this->protocol);
}

const QString           &Request::getMethod() const
{
    return (this->method);
}

void                    Request::setMethod(const QString &method)
{
    this->method = method;
}

const QUrl              &Request::getUri() const
{
    return (this->uri);
}

void                    Request::setUri(const QUrl &uri)
{
    this->uri = uri;
}

const QString           &Request::getVersion() const
{
    return (this->version);
}

void                    Request::setVersion(const QString &version)
{
    this->version = version;
}

const QString           &Request::getType() const
{
    return (this->type);
}

void                    Request::setType(const QString &type)
{
    this->type = type;
}

QVariantList            &Request::getInformations()
{
    return (this->informations);
}

QMap<QString, QString>  &Request::getHeader()
{
    return (this->header);
}

LightBird::IContent     &Request::getContent()
{
    return (this->content);
}

bool                    Request::isError() const
{
    return (this->error);
}

void                    Request::setError(bool error)
{
    this->error = error;
}

void                    Request::clear()
{
    this->protocol.clear();
    this->method.clear();
    this->uri.clear();
    this->version.clear();
    this->type.clear();
    this->informations.clear();
    this->header.clear();
    this->content.clear();
    this->error = false;
}

void                    Request::setProtocol(const QString &protocol)
{
    this->protocol = protocol;
}
