#include <QtPlugin>

#include "QJson/parser.h"
#include "QJson/serializer.h"
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
    metadata.brief = "Serializes and deserializes JSON streams to QVariant type.";
    metadata.description = "Serializes and deserializes JSON streams to QVariant type, which is a generic data representation that can be used independently of the format. This plugin uses QJson to deserialize.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

void    Plugin::onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type)
{
    type = type;
    client = client;
#ifdef QJSON
    LightBird::IContent &content = client.getRequest().getContent();
    bool                ok;

    if (type == LightBird::IOnDeserialize::IDoDeserialize && !client.getRequest().isError() &&
        (client.getRequest().getType().toLower() == "json" || client.getRequest().getType().toLower() == "application/json") &&
        (content.getStorage() == LightBird::IContent::BYTEARRAY || content.getStorage() == LightBird::IContent::TEMPORARYFILE))
    {
        QJson::Parser parser;
        QVariant deserialized = parser.parse(content.getContent(), &ok);
        content.setStorage(LightBird::IContent::VARIANT);
        *content.getVariant() = deserialized;
        if (!ok)
        {
            client.getRequest().setError();
            client.getResponse().setCode(400);
            client.getResponse().setMessage("Bad Request");
            client.getResponse().getContent().setContent(QString("Parse error in the JSon in the content of the request: \"" + parser.errorString() + \
                                                         "\", line: " + QString::number(parser.errorLine()) + ".").toAscii());
        }
    }
#endif // !QJSON
}

bool    Plugin::onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type)
{
    LightBird::IContent  &content = client.getResponse().getContent();
    if (type == LightBird::IOnSerialize::IDoSerialize && !client.getResponse().isError()
        && (client.getResponse().getType().toLower() == "json" || client.getResponse().getType().toLower() == "application/json")
        && content.getStorage() == LightBird::IContent::VARIANT)
    {
#ifdef QJSON
        QJson::Serializer serializer;
        const QByteArray &serialized = serializer.serialize(*content.getVariant());
        content.setStorage(LightBird::IContent::BYTEARRAY);
        content.setContent(serialized);
#else // !QJSON
        const QByteArray &serialized = this->_serializer(*content.getVariant());
        content.setStorage(LightBird::IContent::BYTEARRAY);
        content.setContent(serialized);
#endif
    }
    return (true);
}

QByteArray      Plugin::_serializer(const QVariant &toAnalyze)
{
    QByteArray  result;

    if (!toAnalyze.isValid())
        return (result);
    else if (toAnalyze.type() == QVariant::List)
    {
        QListIterator<QVariant> it(toAnalyze.toList());
        if (it.hasNext())
            result = "[";
        while (it.hasNext())
        {
            result += this->_serializer(it.next());
            if (it.hasNext())
                result += ",";
        }
        if (!result.isEmpty())
            result.append("]");
    }
    else if (toAnalyze.type() == QVariant::Map)
    {
        QMapIterator<QString, QVariant> it(toAnalyze.toMap());
        QStringList name;
        if (it.hasNext())
            result = "{";
        while (it.hasNext())
        {
            it.next();
            // If the key has already been used
            if (name.contains(it.key()))
                continue ;
            // Add the key
            result.append("\"" + _replace(it.key().toAscii()) + "\":");
            // If there is more than one key that have this name
            if (toAnalyze.toMap().count(it.key()) > 1)
                result.append(this->_serializer(toAnalyze.toMap().values(it.key())));
            else
            {
                result.append(this->_serializer(it.value()));
                if (it.hasNext())
                    result.append(",");
            }
            name << it.key();
        }
        if (!result.isEmpty())
            result.append("}");
    }
    else if (toAnalyze.type() == QVariant::StringList)
    {
        QStringListIterator it(toAnalyze.toStringList());
        if (it.hasNext())
            result = "{";
        while (it.hasNext())
        {
            it.next();
            if (it.hasNext())
            {
                if (result.size() != 1)
                    result.append(",");
                result += "\"" + _replace(it.peekPrevious().toAscii()) + "\":\"" + _replace(it.peekNext().toAscii()) + "\"";
                it.next();
            }
        }
        if (!result.isEmpty())
            result.append("}");
    }
    else if (toAnalyze.canConvert(QVariant::String))
        result = "\"" + _replace(toAnalyze.toString().toAscii()) + "\"";
    return (result);
}

QByteArray  Plugin::_replace(QByteArray str)
{
    return (str.replace('\\', "\\\\").replace('"', "\\\"").replace('/', "\\/").replace('\b', "\\b")
            .replace('\f', "\\f").replace('\n', "\\n").replace('\r', "\\r").replace('\t', "\\t"));
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
