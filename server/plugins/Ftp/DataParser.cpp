#include "DataParser.h"

DataParser::DataParser(LightBird::IApi *api, LightBird::IClient *client) : Parser(api, client)
{
}

bool DataParser::doUnserializeContent(const QByteArray &data, quint64 &used)
{
    // Make sure we upload to a file as the content might be big
    client->getRequest().getContent().setStorage(LightBird::IContent::TEMPORARYFILE);
    client->getRequest().getContent().getTemporaryFile()->setAutoRemove(false);
    client->getRequest().getContent().setContent(data, true);
    used += data.size();
    api->log().trace("New data");
    return (client->isDisconnecting());
}

bool DataParser::doSerializeContent(QByteArray &data)
{
    data.append(client->getResponse().getContent().getContent());
    return (true);
}

void DataParser::onFinish()
{
    api->log().trace("Disconnect");
    api->network().disconnect(client->getId());
}

bool DataParser::onSerialize(LightBird::IOnSerialize::Serialize type)
{
    if (type == LightBird::IOnSerialize::IDoSerialize)
        return (false);
    else
        return (true);
}

bool DataParser::onDisconnect()
{
    return (false);
}
