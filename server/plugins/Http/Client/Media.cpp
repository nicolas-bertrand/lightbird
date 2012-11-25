#include <QFileInfo>

#include "Media.h"
#include "Plugin.h"

Media::Media(LightBird::IClient &cl)
    : api(Plugin::api())
    , client(cl)
    , uri(client.getRequest().getUri())
    , response(client.getResponse())
{
    this->error = false;
    this->id = this->client.getInformations().value("sid").toString() + this->uri.queryItemValue("streamId");
    this->response.getContent().setStorage(LightBird::IContent::BYTEARRAY);
    // Extract the file id from the uri
    this->file.setId(this->uri.queryItemValue("id"));
    // If the file doesn't exists, an error occured
    if (!this->file.exists() || !QFileInfo(this->file.getFullPath()).isFile())
    {
        this->_error(404, "Not Found", "File not found.");
        return ;
    }
    // If the client has not the right to read the file
    if (!this->file.isAllowed(this->client.getAccount().getId(), "read"))
    {
        this->_error(403, "Forbidden", "Access to the file denied.");
        return ;
    }
}

Media::~Media()
{
}

bool    Media::isError()
{
    return (this->error);
}

const QString   &Media::getId()
{
    return (this->id);
}

void    Media::_error(int code, const QString &message, const QByteArray &content)
{
    this->response.getContent().setStorage(LightBird::IContent::BYTEARRAY);
    this->response.setCode(code);
    this->response.setMessage(message);
    if (!content.isEmpty())
        this->response.getContent().setData(content);
    this->error = true;
}
