#ifndef REQUEST_H
# define REQUEST_H

# include "Content.h"

# include "IRequest.h"

/// @brief Request is the server implementation of LightBird::IRequest,
/// and represents a client's request.
class Request : public QObject,
                public LightBird::IRequest
{
    Q_OBJECT

public:
    Request(QObject *parent = 0);
    ~Request();

    // IRequest
    const QString           &getProtocol() const;
    const QString           &getMethod() const;
    void                    setMethod(const QString &method);
    const QUrl              &getUri() const;
    void                    setUri(const QUrl &uri);
    const QString           &getVersion() const;
    void                    setVersion(const QString &version);
    const QString           &getType() const;
    void                    setType(const QString &type);
    QVariantList            &getInformations();
    QMap<QString, QString>  &getHeader();
    LightBird::IContent     &getContent();
    bool                    isError() const;
    void                    setError(bool error = true);

    // Other
    /// @brief Clean all the member of the instance.
    void                    clear();
    /// @brief Set the protocol of the request.
    void                    setProtocol(const QString &protocol);

private:
    Request(const Request &);
    Request &operator=(const Request &);

    QString                 protocol;
    QString                 method;
    QUrl                    uri;
    QString                 version;
    QString                 type;
    QVariantList            informations;
    QMap<QString, QString>  header;
    Content                 content;
    bool                    error;
};

#endif // REQUEST_H
