#ifndef MEDIA_H
# define MEDIA_H

# include <QMap>

# include "IApi.h"
# include "IClient.h"
# include "TableFiles.h"

/// @brief Represents a media (audio or video).
class Media
{
public:
    Media(LightBird::IClient &client);
    virtual ~Media();
    /// @brief Reads the data on the transcoding stream and put it in the client's response.
    virtual void    read() = 0;
    /// @brief Returns true if there is an error at some point.
    bool            isError();
    /// @brief Returns the id of the stream.
    const QString   &getId();

protected:
    /// @brief Generates an error
    void    _error(int code, const QString &message, const QByteArray &content);

    LightBird::IApi       &api;       ///< The LightBird's Api.
    LightBird::IClient    &client;    ///< The client that requested the preview.
    QUrl                  uri;        ///< The uri of the request.
    LightBird::TableFiles file;       ///< The file pointed by the uri.
    LightBird::IResponse  &response;  ///< The response that will be sent.
    bool                  error;      ///< True if an error occured at some point.
    QList<void *>         extensions; ///< The list of the extensions that implements IVideo or IAudio.
    QString               id;         ///< The id of the media stream. Used by the client to stop the transcode.
};

#endif // MEDIA_H
