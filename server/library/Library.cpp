#include "Configuration.h"
#include "Identify.h"
#include "Library.h"
#include "Preview.h"
#include "FilesExtensions.h"

using namespace LightBird;

Library *Library::_instance = NULL;

Library *Library::getInstance()
{
    if (!Library::_instance)
        Library::_instance = new Library();
    return (Library::_instance);
}

void Library::initialize()
{
    Library::getInstance()->_identify = new LightBird::Identify();
    Library::getInstance()->_preview = new LightBird::Preview();
    Library::getInstance()->_filesExtensions = new LightBird::FilesExtensions();
}

void Library::shutdown()
{
    delete Library::_instance;
    Library::_instance = NULL;
}

Library::Library()
    : _configuration(NULL)
    , _database(NULL)
    , _extension(NULL)
    , _log(NULL)
    , _c(NULL)
    , _identify(NULL)
    , _preview(NULL)
    , _filesExtensions(NULL)
{
    _imageExtensions.insert(LightBird::IImage::BMP, "bmp");
    _imageExtensions.insert(LightBird::IImage::GIF, "gif");
    _imageExtensions.insert(LightBird::IImage::JPEG, "jpeg");
    _imageExtensions.insert(LightBird::IImage::PNG, "png");
    _imageExtensions.insert(LightBird::IImage::TGA, "tga");
    _imageExtensions.insert(LightBird::IImage::TIFF, "tiff");
}

Library::~Library()
{
    delete _c;
    delete _identify;
    delete _preview;
    delete _filesExtensions;
    // This IDatabase and ILogs were allocated just for us
    delete _database;
    delete _log;
}

void Library::setConfiguration(IConfiguration *configuration)
{
    if (!(_instance = Library::getInstance())->_configuration)
    {
        Library::getInstance()->_configuration = configuration;
        Library::_instance->_c = new Configuration(configuration);
    }
}

void Library::setDatabase(IDatabase *database)
{
    if (!(_instance = Library::getInstance())->_database)
        Library::getInstance()->_database = database;
}

void Library::setExtension(IExtensions *extension)
{
    if (!(_instance = Library::getInstance())->_extension)
        Library::getInstance()->_extension = extension;
}

void Library::setLog(ILogs *log)
{
    if (!(_instance = Library::getInstance())->_log)
        Library::getInstance()->_log = log;
}

IConfiguration  &Library::configuration()
{
    return (*Library::_instance->_configuration);
}

IDatabase   &Library::database()
{
    return (*Library::_instance->_database);
}

IExtensions &Library::extension()
{
    return (*Library::_instance->_extension);
}

ILogs   &Library::log()
{
    return (*Library::_instance->_log);
}

LightBird::Identify *Library::getIdentify()
{
    return (Library::_instance->_identify);
}

LightBird::Preview *Library::getPreview()
{
    return (Library::_instance->_preview);
}

LightBird::FilesExtensions *Library::getFilesExtensions()
{
    return (Library::_instance->_filesExtensions);
}

QHash<LightBird::IImage::Format, QString> &Library::getImageExtensions()
{
    return (Library::_instance->_imageExtensions);
}
