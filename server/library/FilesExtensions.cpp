#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "Configuration.h"
#include "Properties.h"
#include "Library.h"
#include "FilesExtensions.h"

FilesExtensions::FilesExtensions()
    : _defaultMime("application/octet-stream")
{
    QFile file(LightBird::Configuration::get().filesExtensionsPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        LightBird::Library::log().error("Unable to open the files extensions JSON", Properties("path", LightBird::Configuration::get().filesExtensionsPath).toMap(), "FilesExtensions", "FilesExtensions");
        return ;
    }
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (document.isNull())
    {
        LightBird::Library::log().error("Unable to load the JSON filesExtensions", Properties("path", LightBird::Configuration::get().filesExtensionsPath).add("error", error.errorString()).add("offset", error.offset).toMap(), "FilesExtensions", "FilesExtensions");
        return ;
    }

    QJsonObject object = document.object();
    for (QJsonObject::iterator it = object.begin(); it != object.end(); ++it)
    {
        Data d;
        QString type = it.value().toObject().value("type").toString().toLower();
        if (type == "audio")
            d.type = LightBird::IIdentify::AUDIO;
        else if (type == "document")
            d.type = LightBird::IIdentify::DOCUMENT;
        else if (type == "image")
            d.type = LightBird::IIdentify::IMAGE;
        else if (type == "video")
            d.type = LightBird::IIdentify::VIDEO;
        else
            d.type = LightBird::IIdentify::OTHER;
        if ((d.mime = it.value().toObject().value("mime").toString()).isEmpty())
            d.mime = _defaultMime;
        _extensions.insert(it.key(), d);
    }
}

FilesExtensions::~FilesExtensions()
{
}

LightBird::IIdentify::Type FilesExtensions::getFileType(const QString &fileName)
{
    int index;
    Data defaultValue;
    defaultValue.type = LightBird::IIdentify::OTHER;

    if ((index = fileName.lastIndexOf('.')) >= 0)
        return _extensions.value(fileName.right(fileName.size() - index - 1), defaultValue).type;
    else
        return _extensions.value(fileName, defaultValue).type;
}

QString FilesExtensions::getFileMime(const QString &fileName)
{
    int index;
    Data defaultValue;
    defaultValue.mime = _defaultMime;

    if ((index = fileName.lastIndexOf('.')) >= 0)
        return _extensions.value(fileName.right(fileName.size() - index - 1), defaultValue).mime;
    else
        return _extensions.value(fileName, defaultValue).mime;
}
