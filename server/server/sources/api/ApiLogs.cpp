#include "ApiLogs.h"
#include "Log.h"

ApiLogs::ApiLogs(const QString &id, bool plugin)
{
    this->id = id;
    this->plugin = plugin;
}

ApiLogs::~ApiLogs()
{
}

void    ApiLogs::write(LightBird::ILogs::Level level, const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method) const
{
    Log::instance()->write(level, message, (this->plugin ? this->id : properties.value("plugin")), properties, className, method);
}

void    ApiLogs::fatal(const QString &message, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::FATAL, message, this->id, QMap<QString, QString>(), className, method);
}

void    ApiLogs::fatal(const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::FATAL, message, (this->plugin ? this->id : properties.value("plugin")), properties, className, method);
}

void    ApiLogs::error(const QString &message, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::ERROR, message, this->id, QMap<QString, QString>(), className, method);
}

void    ApiLogs::error(const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::ERROR, message, (this->plugin ? this->id : properties.value("plugin")), properties, className, method);
}

void    ApiLogs::warning(const QString &message, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::WARNING, message, this->id, QMap<QString, QString>(), className, method);
}

void    ApiLogs::warning(const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::WARNING, message, (this->plugin ? this->id : properties.value("plugin")), properties, className, method);
}

void    ApiLogs::info(const QString &message, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::INFO, message, this->id, QMap<QString, QString>(), className, method);
}

void    ApiLogs::info(const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::INFO, message, (this->plugin ? this->id : properties.value("plugin")), properties, className, method);
}

void    ApiLogs::debug(const QString &message, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::DEBUG, message, this->id, QMap<QString, QString>(), className, method);
}

void    ApiLogs::debug(const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::DEBUG, message, (this->plugin ? this->id : properties.value("plugin")), properties, className, method);
}

void    ApiLogs::trace(const QString &message, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::TRACE, message, this->id, QMap<QString, QString>(), className, method);
}

void    ApiLogs::trace(const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method) const
{
    Log::instance()->write(LightBird::ILogs::TRACE, message, (this->plugin ? this->id : properties.value("plugin")), properties, className, method);
}

LightBird::ILogs::Level ApiLogs::getLevel() const
{
    return (Log::instance()->getLevel());
}

void    ApiLogs::setLevel(LightBird::ILogs::Level level)
{
    Log::instance()->setLevel(level);
}

bool    ApiLogs::isDisplay() const
{
    return (Log::instance()->isDisplay());
}

void    ApiLogs::isDisplay(bool display)
{
    Log::instance()->isDisplay(display);
}

bool    ApiLogs::isError() const
{
    return (Log::instance()->isError());
}

bool    ApiLogs::isWarning() const
{
    return (Log::instance()->isWarning());
}

bool    ApiLogs::isInfo() const
{
    return (Log::instance()->isInfo());
}

bool    ApiLogs::isDebug() const
{
    return (Log::instance()->isDebug());
}

bool    ApiLogs::isTrace() const
{
    return (Log::instance()->isTrace());
}
