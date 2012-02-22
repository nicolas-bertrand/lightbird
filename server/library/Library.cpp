#include "Library.h"

LightBird::IConfiguration *LightBird::Library::_configuration = NULL;
LightBird::IDatabase      *LightBird::Library::_database = NULL;
LightBird::ILogs          *LightBird::Library::_log = NULL;

LightBird::IConfiguration &LightBird::Library::configuration()
{
    return (*LightBird::Library::_configuration);
}

LightBird::IDatabase &LightBird::Library::database()
{
    return (*LightBird::Library::_database);
}

LightBird::ILogs &LightBird::Library::log()
{
    return (*LightBird::Library::_log);
}

void LightBird::Library::setConfiguration(LightBird::IConfiguration *configuration)
{
    if (!LightBird::Library::_configuration)
        LightBird::Library::_configuration = configuration;
}

void LightBird::Library::setDatabase(LightBird::IDatabase *database)
{
    if (!LightBird::Library::_database)
        LightBird::Library::_database = database;
}

void LightBird::Library::setLog(LightBird::ILogs *log)
{
    if (!LightBird::Library::_log)
        LightBird::Library::_log = log;
}
