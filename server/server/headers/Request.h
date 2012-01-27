#ifndef REQUEST_H
# define REQUEST_H

# include <QObject>

# include "Content.h"

# include "IRequest.h"

/// @brief The server implementation of LightBird::IRequest.
class Request : public QObject,
                public LightBird::IRequest
{
    Q_OBJECT

public:
    Request(QObject *parent = NULL);
    ~Request();

    // LightBird::IRequest
    const QString           &getProtocol() const;
    const QString           &getMethod() const;
    void                    setMethod(const QString &method);
    const QUrl              &getUri() const;
    void                    setUri(const QUrl &uri);
    const QString           &getVersion() const;
    void                    setVersion(const QString &version);
    const QString           &getType() const;
    void                    setType(const QString &type);
    QVariantMap             &getInformations();
    QMap<QString, QString>  &getHeader();
    LightBird::IContent     &getContent();
    bool                    isError() const;
    void                    setError(bool error = true);

    // Other
    /// @brief Cleans all the member of the instance.
    void                    clear();
    /// @brief Sets the protocol of the request.
    void                    setProtocol(const QString &protocol);

private:
    Request(const Request &);
    Request &operator=(const Request &);

    QString                 protocol;
    QString                 method;
    QUrl                    uri;
    QString                 version;
    QString                 type;
    QVariantMap             informations;
    QMap<QString, QString>  header;
    Content                 content;
    bool                    error;
};

#endif // REQUEST_H
