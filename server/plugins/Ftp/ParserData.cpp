#include "ParserData.h"
#include "Plugin.h"

ParserData::ParserData(LightBird::IApi *api, LightBird::IClient &client) : Parser(api, client)
{
}

ParserData::~ParserData()
{
}

bool    ParserData::doUnserializeContent(const QByteArray &data, quint64 &)
{
    // Upload in progress (otherwise the data received are just discarded)
    if (this->client.getInformations().contains("upload"))
        this->client.getRequest().getContent().setContent(data);
    return (false);
}

bool    ParserData::doSerializeContent(QByteArray &data)
{
    LightBird::IContent &content = this->client.getResponse().getContent();
    quint32 maxPacketSize = Plugin::getConfiguration().maxPacketSize;
    bool    result = true;

    if (content.size() < maxPacketSize)
        data = content.getContent();
    else
    {
        data = content.getContent(maxPacketSize);
        result = (content.getSeek() >= content.size());
    }
    if (result)
        this->client.getInformations().insert("download-completed", true);
    return (result);
}

void    ParserData::onFinish()
{
    this->api->network().disconnect(this->client.getId());
}

bool    ParserData::onSerialize(LightBird::IOnSerialize::Serialize)
{
    // The response to the client is not needed in CLIENT mode
    return (false);
}

bool    ParserData::onDisconnect()
{
    LightBird::Session  session = this->client.getSession();

    // The data connection has been aborted
    if (session && session->getInformation("disconnect-data").toBool())
        return (true);
    // If the client is downloading data we can close the connection directly.
    // Otherwise we have to wait that the upload is finished.
    return (this->client.getInformations().contains("download"));
}

void    ParserData::onDestroy()
{
    LightBird::Session  session = this->client.getSession();
    QVariantMap         &informations = this->client.getInformations();

    // Destroy the session if there is no control connection
    if (session)
    {
        // If we just uploaded a file, we have to identify it
        if (informations.contains("upload") && informations.contains("upload-id"))
            Plugin::identify(informations.value("upload-id").toString());
        // If the download was not completed before the disconnection, an error occured
        if (informations.contains("download") && informations.contains("message") && !informations.contains("download-completed"))
            Plugin::sendControlMessage(session->getInformation("control-id").toString(), Commands::Result(426, "Transfer aborted."));
        // A message have to be sent after the transfer
        else if (informations.contains("message"))
            Plugin::sendControlMessage(session->getInformation("control-id").toString(), Commands::Result(informations.value("code").toUInt(), informations.value("message").toString()));
        session->setInformation("disconnect-data", false);
        session->removeInformation("data-id");
        session->removeClient(this->client.getId());
        if (session->getInformation("control-id").toString().isEmpty())
            session->destroy();
    }
}
