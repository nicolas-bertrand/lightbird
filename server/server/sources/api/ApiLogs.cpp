#include "Log.h"
#include "ApiLogs.h"

ApiLogs::ApiLogs(const QString &id, QObject *parent) : QObject(parent)
{
    this->id = id;
}

ApiLogs::~ApiLogs()
{
    Log::trace("ApiLogs destroyed!", Properties("id", this->id), "ApiLogs", "~ApiLogs");
}

void    ApiLogs::write(LightBird::ILogs::level level, const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method)
{
    Log::instance()->write(level, message, this->id, properties, className, method);
}

void    ApiLogs::fatal(const QString &message, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::FATAL, message, this->id, QMap<QString, QString>(), className,method);
}

void    ApiLogs::fatal(const QString &message, QMap<QString, QString> &properties, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::FATAL, message, this->id, properties, className,method);
}

void    ApiLogs::error(const QString &message, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::ERROR, message, this->id, QMap<QString, QString>(), className, method);
}

void    ApiLogs::error(const QString &message, QMap<QString, QString> &properties, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::ERROR, message, this->id, properties, className,method);
}

void    ApiLogs::warning(const QString &message, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::WARNING, message, this->id, QMap<QString, QString>(), className,method);
}

void    ApiLogs::warning(const QString &message, QMap<QString, QString> &properties, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::WARNING, message, this->id, properties, className,method);
}

void    ApiLogs::info(const QString &message, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::INFO, message, this->id, QMap<QString, QString>(), className,method);
}

void    ApiLogs::info(const QString &message, QMap<QString, QString> &properties, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::INFO, message, this->id, properties, className,method);
}

void    ApiLogs::debug(const QString &message, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::DEBUG, message, this->id, QMap<QString, QString>(), className,method);
}

void    ApiLogs::debug(const QString &message, QMap<QString, QString> &properties, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::DEBUG, message, this->id, properties, className,method);
}

void    ApiLogs::trace(const QString &message, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::TRACE, message, this->id, QMap<QString, QString>(), className, method);
}

void    ApiLogs::trace(const QString &message, QMap<QString, QString> &properties, const QString &className, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::TRACE, message, this->id, properties, className,method);
}

LightBird::ILogs::level ApiLogs::getlevel()
{
    return (Log::instance()->getlevel());
}

void    ApiLogs::setLevel(LightBird::ILogs::level level)
{
    Log::instance()->setLevel(level);
}

bool    ApiLogs::isError()
{
    return (Log::instance()->isError());
}

bool    ApiLogs::isWarning()
{
    return (Log::instance()->isWarning());
}

bool    ApiLogs::isInfo()
{
    return (Log::instance()->isInfo());
}

bool    ApiLogs::isDebug()
{
    return (Log::instance()->isDebug());
}

bool    ApiLogs::isTrace()
{
    return (Log::instance()->isTrace());
}
