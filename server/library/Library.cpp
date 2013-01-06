#include "Identify.h"
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

void Library::initialize()
{
    Library::getInstance()->identify = new Identify();
    Library::getInstance()->preview = new Preview();
}

void Library::shutdown()
{
    delete Library::instance;
    Library::instance = NULL;
}

Library::Library()
    : _configuration(NULL)
    , _database(NULL)
    , _extension(NULL)
    , _log(NULL)
    , identify(NULL)
    , preview(NULL)
{
    this->imageExtensions.insert(LightBird::IImage::BMP, "bmp");
    this->imageExtensions.insert(LightBird::IImage::GIF, "gif");
    this->imageExtensions.insert(LightBird::IImage::JPEG, "jpeg");
    this->imageExtensions.insert(LightBird::IImage::PNG, "png");
    this->imageExtensions.insert(LightBird::IImage::TGA, "tga");
    this->imageExtensions.insert(LightBird::IImage::TIFF, "tiff");
}

Library::~Library()
{
    // This ILogs was allocated just for us
    delete this->_log;
    delete this->identify;
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

Identify *Library::getIdentify()
{
    return (Library::instance->identify);
}

Preview *Library::getPreview()
{
    return (Library::instance->preview);
}

QHash<LightBird::IImage::Format, QString> &Library::getImageExtensions()
{
    return (Library::instance->imageExtensions);
}
