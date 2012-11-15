#include "Library.h"
#include "Preview.h"

using namespace LightBird;

Library *Library::instance = NULL;

Library *Library::getInstance()
{
    if (!Library::instance)
        Library::instance = new Library();
    return (Library::instance);
}

void Library::shutdown()
{
    delete Library::instance;
    Library::instance = NULL;
}

Library::Library() : _configuration(NULL),
                     _database(NULL),
                     _extension(NULL),
                     _log(NULL),
                     preview(NULL)
{
}

Library::~Library()
{
    // This ILogs was allocated just for us
    delete this->_log;
    delete this->preview;
}

void Library::setConfiguration(IConfiguration *configuration)
{
    if (!(instance = Library::getInstance())->_configuration)
        Library::getInstance()->_configuration = configuration;
}

void Library::setDatabase(IDatabase *database)
{
    if (!(instance = Library::getInstance())->_database)
        Library::getInstance()->_database = database;
}

void Library::setExtension(IExtensions *extension)
{
    if (!(instance = Library::getInstance())->_extension)
        Library::getInstance()->_extension = extension;
}

void Library::setLog(ILogs *log)
{
    if (!(instance = Library::getInstance())->_log)
        Library::getInstance()->_log = log;
}

IConfiguration  &Library::configuration()
{
    return (*Library::instance->_configuration);
}

IDatabase   &Library::database()
{
    return (*Library::instance->_database);
}

IExtensions &Library::extension()
{
    return (*Library::instance->_extension);
}

ILogs   &Library::log()
{
    return (*Library::instance->_log);
}

Preview *Library::getPreview()
{
    if (!Library::instance->preview)
        Library::instance->preview = new Preview();
    return (Library::instance->preview);
}
