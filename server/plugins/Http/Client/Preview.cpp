#include <QFileInfo>
#include <QUrlQuery>
#include "LightBird.h"

#include "Preview.h"

Preview::Preview(LightBird::IClient &cl)
    : client(cl)
    , uri(client.getRequest().getUri())
    , response(client.getResponse())
{
}

Preview::~Preview()
{
}

void    Preview::generate()
{
    unsigned int position;
    float        quality;
    QUrlQuery    query(this->uri);

    // Extracts the file id from the uri
    this->file.setId(query.queryItemValue("fileId"));
    // If the file does not exist, an error occured
    if (!this->file.exists() || !QFileInfo(this->file.getFullPath()).isFile())
        return this->_error("Preview", 404, "Not Found", "File not found.");
    // If the client has not the right to read the file
    if (!this->file.isAllowed(this->client.getAccount().getId(), "read"))
        return this->_error("Preview", 403, "Forbidden", "Access to the file denied.");
    // Defines the width and the height of the image
    this->_size();
    // Gets the time and the quality of the preview
    position = query.queryItemValue("position").toUInt();
    if (!(quality = query.queryItemValue("quality").toFloat()))
        quality = -1;
    // Generates a preview of the file
    this->previewFileName = LightBird::preview(this->file.getId(), LightBird::IImage::JPEG, this->width, this->height, position, quality);
    // No extensions has been able to generate the preview
    if (this->previewFileName.isEmpty())
    {
        if (Plugin::api().log().isTrace())
            Plugin::api().log().trace("Unable to generate a preview from this file.", Properties("fileId", this->file.getId()).toMap(), "Preview", "generate");
        return this->_error("Preview", 404, "Not Found", "Unable to generate a preview from this file.");
    }
    // Put the preview in the response
    this->response.getContent().setStorage(LightBird::IContent::FILE, this->previewFileName);
    this->response.setType("image/jpeg");
}

void    Preview::_size()
{
    this->width = QUrlQuery(this->uri).queryItemValue("width").toUInt();
    this->height = QUrlQuery(this->uri).queryItemValue("height").toUInt();
    if (!this->width && !this->height)
    {
        this->width = 100;
        this->height = 75;
    }
    if (this->width > 800)
        this->width = 800;
    if (this->height > 600)
        this->height = 600;
}

void    Preview::_error(const QString &method, int code, const QString &message,
                        const QByteArray &content, const QString &log, LightBird::ILogs::Level level)
{
    this->response.getContent().setStorage(LightBird::IContent::BYTEARRAY);
    this->response.setCode(code);
    this->response.setMessage(message);
    if (!content.isEmpty())
        this->response.getContent().setData(content);
    if (!log.isEmpty())
        Plugin::api().log().write(level, log, Properties("fileId", this->file.getId()).toMap(), "Preview", method);
}
