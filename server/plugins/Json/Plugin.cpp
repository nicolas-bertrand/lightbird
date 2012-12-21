#include <QJsonDocument>

#include "Plugin.h"

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUnload()
{
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    this->api = api;
}

void    Plugin::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "Json parser";
    metadata.brief = "Serializes and deserializes JSON data to QVariant type.";
    metadata.description = "Serializes and deserializes JSON streams to QVariant type, which is a generic data representation that can be used independently of the format.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

void    Plugin::onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type)
{
    LightBird::IContent &content = client.getRequest().getContent();

    if (type == LightBird::IOnDeserialize::IDoDeserialize && !client.getRequest().isError() &&
        (client.getRequest().getType().toLower() == "json" || client.getRequest().getType().toLower() == "application/json") &&
        (content.getStorage() == LightBird::IContent::BYTEARRAY || content.getStorage() == LightBird::IContent::TEMPORARYFILE))
    {
        QJsonParseError error;
        QVariant deserialized = QJsonDocument::fromJson(content.getData(), &error);
        if (!error.error == QJsonParseError::NoError)
        {
            content.setStorage(LightBird::IContent::VARIANT);
            *content.getVariant() = deserialized;
        }
        else
        {
            client.getRequest().setError();
            client.getResponse().setCode(400);
            client.getResponse().setMessage("Bad Request");
            client.getResponse().getContent().setData(
                        QString("Parse error in the JSON in the content of the request: \"" + error.errorString() +
                                "\", offset: " + QString::number(error.offset) + ".").toLatin1());
        }
    }
}

bool    Plugin::onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type)
{
    LightBird::IContent  &content = client.getResponse().getContent();
    if (type == LightBird::IOnSerialize::IDoSerialize && !client.getResponse().isError()
        && (client.getResponse().getType().toLower() == "json" || client.getResponse().getType().toLower() == "application/json")
        && content.getStorage() == LightBird::IContent::VARIANT)
    {
        QByteArray json = content.getVariant()->toJsonDocument().toJson();
        content.setStorage(LightBird::IContent::BYTEARRAY);
        content.setData(json);
    }
    return (true);
}
