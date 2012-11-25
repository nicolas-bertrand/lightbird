#ifndef PREVIEW_H
# define PREVIEW_H

# include <QMap>

# include "IApi.h"
# include "IClient.h"
# include "ILog.h"
# include "IResponse.h"

# include "Plugin.h"
# include "TableFiles.h"

class Preview
{
public:
    Preview(LightBird::IClient &client);
    ~Preview();
    void    go();

private:
    /// @brief Returns the size of the image depending on the size requested by the client.
    void    _size();
    /// @brief Generated an error
    void    _error(const QString &method, int code, const QString &message, const QByteArray &content = "",
                   const QString &log = "", LightBird::ILogs::Level level = LightBird::ILogs::WARNING);

    LightBird::IClient   &client;        ///< The client that requested the preview.
    QUrl                 uri;            ///< The uri of the request.
    LightBird::TableFiles file;          ///< The file pointed by the uri.
    LightBird::IResponse &response;      ///< The response that will be sent.
    QString              previewFileName;///< The name of the preview file.
    unsigned int         width;          ///< The width of the preview.
    unsigned int         height;         ///< The height of the preview.
    QMap<QString, QString> properties;   ///< Properties used for the logs.
};

#endif // PREVIEW_H
