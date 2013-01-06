#include "Ftp.h"
#include "ParserData.h"
#include "Plugin.h"

ParserData::ParserData(LightBird::IApi &api, LightBird::IClient &client)
    : Parser(api, client)
{
}

ParserData::~ParserData()
{
}

bool    ParserData::doDeserializeContent(const QByteArray &data, quint64 &)
{
    // Upload in progress (otherwise the data received are just discarded)
    if (this->client.getInformations().contains(DATA_UPLOAD))
        this->client.getRequest().getContent().setData(data);
    return (false);
}

bool    ParserData::doSerializeContent(QByteArray &data)
{
    LightBird::IContent &content = this->client.getResponse().getContent();
    quint32 maxPacketSize = Plugin::getConfiguration().maxPacketSize;
    bool    result = true;

    if (content.size() < maxPacketSize)
        data = content.getData();
    else
    {
        data = content.getData(maxPacketSize);
        result = (content.getSeek() >= content.size());
    }
    if (result)
        this->client.getInformations().insert(DATA_DOWNLOAD_COMPLETED, true);
    return (result);
}
