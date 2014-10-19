#include <QFileInfo>

#include "Library.h"
#include "LightBird.h"
#include "Configuration.h"

LightBird::Configuration *LightBird::Configuration::_instance = NULL;

LightBird::Configuration::Configuration(LightBird::IConfiguration *configuration)
    : _c(*configuration)
{
    if (!_instance)
        _instance = this;
    _update();
}

LightBird::Configuration::~Configuration()
{
    _instance = NULL;
}

void LightBird::Configuration::_update()
{
    name = _c.get("name");
    pluginsPath = _c.get("pluginsPath", DEFAULT_PLUGINS_PATH);
    QtPluginsPath = _c.get("QtPluginsPath", DEFAULT_QT_PLUGINS_PATH);
    filesPath = _c.get("filesPath", DEFAULT_FILES_PATH);
    filesExtensionsPath = _c.get("filesExtensionsPath", FILES_EXTENSIONS_RESOURCES_PATH);
    if (!QFileInfo(filesExtensionsPath).isFile())
        filesExtensionsPath = FILES_EXTENSIONS_RESOURCES_PATH;
    temporaryPath = _c.get("temporaryPath", DEFAULT_TEMPORARY_PATH);
    cleanTemporaryPath = _c.get("cleanTemporaryPath", DEFAULT_CLEAN_TEMPORARY_PATH) == "true";
    languagesPath = _c.get("languagesPath", DEFAULT_LANGUAGES_PATH);
    language = _c.get("language");
    threadsNumber = _c.get("threadsNumber", "10").toUInt();
    hashSizeLimit = _c.get("hashSizeLimit", "-1").toLongLong();

    database.name = _c.get("database/name");
    database.file = _c.get("database/file", DEFAULT_DATABASE_FILE);
    database.path = _c.get("database/path", DEFAULT_DATABASE_PATH);
    database.resource = _c.get("database/resource", DEFAULT_DATABASE_RESOURCE);
    database.type = _c.get("database/type", DEFAULT_DATABASE_TYPE);
    database.password = _c.get("database/password");
    database.user = _c.get("database/user");
    database.host = _c.get("database/host");
    database.port = _c.get("database/port").toUShort();

    permissions.activate = _c.get("permissions/activate", DEFAULT_PERMISSIONS_ACTIVATE) == "true";
    permissions.default_ = _c.get("permissions/default", DEFAULT_PERMISSIONS_DEFAULT) == "true";
    permissions.inheritance = _c.get("permissions/inheritance", DEFAULT_PERMISSIONS_INHERITANCE) == "true";
    permissions.ownerInheritance = _c.get("permissions/ownerInheritance", DEFAULT_PERMISSIONS_OWNERINHERITANCE) == "true";
    permissions.groupInheritance = _c.get("permissions/groupInheritance", DEFAULT_PERMISSIONS_GROUPINHERITANCE) == "true";

    log.level = Library::log().getLevelFromString(_c.get("log/level", "info"));
    log.display = _c.get("log/display", "false") == "true";
    log.file = _c.get("log/file", "server.log");
    log.path = _c.get("log/path", "logs");
    log.maxNbOfFile = _c.get("log/maxNbOfFile", "10").toInt();
    log.maxSize = stringToBytes(_c.get("log/maxSize", "1M"));
    log.expires = _c.get("log/expires", "30").toUInt();

    preview.cacheEnabled = _c.get("preview/cacheEnabled", "false") == "true";
    preview.cachePath = _c.get("preview/cachePath", "cache");
    preview.cacheSizeLimit = _c.get("preview/cacheSizeLimit", "-1").toLongLong();
}

void LightBird::Configuration::_set(const QString &key, const QString &value)
{
    int slash = key.indexOf('/');

    if (slash < 0)
    {
        if (key == "name")
            name = value;
        else if (key == "pluginsPath")
            pluginsPath = value;
        else if (key == "QtPluginsPath")
            QtPluginsPath = value;
        else if (key == "filesPath")
            filesPath = value;
        else if (key == "filesExtensionsPath")
            filesExtensionsPath = value;
        else if (key == "temporaryPath")
            temporaryPath = value;
        else if (key == "cleanTemporaryPath")
            cleanTemporaryPath = (value == "true");
        else if (key == "languagesPath")
            languagesPath = value;
        else if (key == "language")
            language = value;
        else if (key == "threadsNumber")
            threadsNumber = value.toUInt();
        else if (key == "hashSizeLimit")
            hashSizeLimit = value.toLongLong();
    }
    else if (slash == key.lastIndexOf('/'))
    {
        QString k = key.right(key.size() - slash - 1);
        if (key.startsWith("database/"))
        {
            if (k == "name")
                database.name = value;
            else if (k == "file")
                database.file = value;
            else if (k == "path")
                database.path = value;
            else if (k == "resource")
                database.resource = value;
            else if (k == "type")
                database.type = value;
            else if (k == "password")
                database.password = value;
            else if (k == "user")
                database.user = value;
            else if (k == "host")
                database.host = value;
            else if (k == "port")
                database.port = value.toUShort();
        }
        else if (key.startsWith("permissions/"))
        {
            if (k == "activate")
                permissions.activate = (value == "true");
            else if (k == "default")
                permissions.default_ = (value == "true");
            else if (k == "inheritance")
                permissions.inheritance = (value == "true");
            else if (k == "ownerInheritance")
                permissions.ownerInheritance = (value == "true");
            else if (k == "groupInheritance")
                permissions.groupInheritance = (value == "true");
        }
        else if (key.startsWith("log/"))
        {
            if (k == "level")
                log.level = Library::log().getLevelFromString(value);
            else if (k == "display")
                log.display = (value == "true");
            else if (k == "file")
                log.file = value;
            else if (k == "path")
                log.path = value;
            else if (k == "maxNbOfFile")
                log.maxNbOfFile = value.toInt();
            else if (k == "maxSize")
                log.maxSize = stringToBytes(value);
            else if (k == "expires")
                log.expires = value.toUInt();
        }
        else if (key.startsWith("preview/"))
        {
            if (k == "cacheEnabled")
                preview.cacheEnabled = (value == "true");
            else if (k == "cachePath")
                preview.cachePath = value;
            else if (k == "cacheSizeLimit")
                preview.cacheSizeLimit = value.toLongLong();
        }
    }
}
