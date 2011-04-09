#include "Response.h"

Response::Response(QObject *parent) : QObject(parent)
{
    this->clear();
}

Response::~Response()
{
}

const QString           &Response::getVersion() const
{
    return (this->version);
}

void                    Response::setVersion(const QString &version)
{
    this->version = version;
}

int                     Response::getCode() const
{
    return (this->code);
}

void                    Response::setCode(int code)
{
    this->code = code;
}

const QString           &Response::getMessage() const
{
    return (this->message);
}

void                    Response::setMessage(const QString &message)
{
    this->message = message;
}

const QString           &Response::getType() const
{
    return (this->type);
}

void                    Response::setType(const QString &type)
{
    this->type = type;
}

QVariantList            &Response::getInformations()
{
    return (this->informations);
}

QMap<QString, QString>  &Response::getHeader()
{
    return (this->header);
}

LightBird::IContent     &Response::getContent()
{
    return (this->content);
}

bool                    Response::getError() const
{
    return (this->error);
}

void                    Response::setError(bool error)
{
    this->error = error;
}

void                    Response::clear()
{
    this->version.clear();
    this->code = 0;
    this->message.clear();
    this->type.clear();
    this->informations.clear();
    this->header.clear();
    this->content.clear();
    this->error = false;
}
