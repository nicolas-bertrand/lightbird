#include <QFileInfo>

#include "ITableFiles.h"

#include "Preview.h"

Preview::Preview(LightBird::IApi &a, LightBird::IClient &cl) : api(a), client(cl), uri(client.getRequest().getUri()),
                 file(api.database().getFiles()), response(client.getResponse())
{
}

Preview::~Preview()
{
}

void    Preview::go()
{
    QList<void *>   extensions;
    unsigned int    position;

    // Extract the file id from the uri
    this->file->setId(this->uri.queryItemValue("id"));
    // If the file doesn't exists, an error occured
    if (!this->file->exists() || !QFileInfo(this->file->getFullPath()).isFile())
        return this->_error("Preview", 404, "Not Found", "File not found.");
    // If the client has not the right to read the file
    if (!this->file->isAllowed(this->client.getAccount().getId(), "read"))
        return this->_error("Preview", 403, "Forbidden", "Access to the file denied.");
    // Defines the width and the height of the image
    this->_size();
    // Get the position of the preview
    position = this->uri.queryItemValue("position").toUInt();
    // Get the extensions that can generates a preview of the file
    if ((extensions = this->api.extensions().get("IPreviews")).size() > 0)
        this->previewFileName = static_cast<LightBird::IPreviews *>(extensions.first())->previews(this->file->getId(), LightBird::IImage::JPEG, this->width, this->height, position);
    // Release the extensions
    this->api.extensions().release(extensions);
    // No extensions has been able to generate the preview
    if (this->previewFileName.isEmpty())
        return this->_error("Preview", 501, "Not Implemented", "Unable to generate a preview from this file.");
    // Put the preview in the response
    this->response.getContent().setStorage(LightBird::IContent::FILE, this->previewFileName);
    this->response.setType("image/jpeg");
}

void    Preview::_size()
{
    this->width = this->uri.queryItemValue("width").toUInt();
    this->height = this->uri.queryItemValue("height").toUInt();
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
        this->response.getContent().setContent(content);
    if (!log.isEmpty())
        this->api.log().write(level, log, this->properties, "Preview", method);
}