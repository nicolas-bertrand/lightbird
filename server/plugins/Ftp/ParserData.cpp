#include "ParserData.h"
#include "Plugin.h"

ParserData::ParserData(LightBird::IApi *api, LightBird::IClient &client) : Parser(api, client)
{
}

ParserData::~ParserData()
{
}

bool ParserData::doUnserializeContent(const QByteArray &data, quint64 &used)
{
    // Make sure we upload to a file as the content might be big
    this->client.getRequest().getContent().setStorage(LightBird::IContent::TEMPORARYFILE);
    this->client.getRequest().getContent().getTemporaryFile()->setAutoRemove(false);
    this->client.getRequest().getContent().setContent(data, true);
    used += data.size();
    return (this->client.isDisconnecting());
}

bool ParserData::doSerializeContent(QByteArray &data)
{
    LightBird::IContent &content = this->client.getResponse().getContent();
    quint32 maxPacketSize = Plugin::getConfiguration().maxPacketSize;

    if (content.size() < maxPacketSize)
        data = content.getContent();
    else
    {
        data = content.getContent(maxPacketSize);
        return (content.getSeek() >= content.size());
    }
    return (true);
}

void ParserData::onFinish()
{
    this->api->network().disconnect(this->client.getId());
}

bool ParserData::onSerialize(LightBird::IOnSerialize::Serialize)
{
    return (false);
}

bool ParserData::onDisconnect()
{
    LightBird::Session  session = this->client.getSession();
    bool                result = false;

    // Destroy the session if there is no control connection
    if (session)
    {
        result = session->getInformation("disconnect-data").toBool();
        session->removeInformation("disconnect-data");
        session->removeInformation("data-id");
        session->removeClient(this->client.getId());
        if (session->getInformation("control-id").toString().isEmpty())
            session->destroy();
    }
    return (result);
}
