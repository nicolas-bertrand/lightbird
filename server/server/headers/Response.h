#ifndef RESPONSE_H
# define RESPONSE_H

# include "Content.h"
# include "QObject.h"

# include "IResponse.h"

/// @brief Response is the server implementation of LightBird::IResponse,
/// and represents a client's response.
class Response : public QObject,
                 public LightBird::IResponse
{
    Q_OBJECT

public:
    Response(QObject *parent = 0);
    ~Response();

    // LightBird::IResponse
    const QString           &getVersion() const;
    void                    setVersion(const QString &version);
    int                     getCode() const;
    void                    setCode(int code);
    const QString           &getMessage() const;
    void                    setMessage(const QString &message);
    const QString           &getType() const;
    void                    setType(const QString &type);
    QVariantList            &getInformations();
    QMap<QString, QString>  &getHeader();
    LightBird::IContent     &getContent();
    bool                    getError() const;
    void                    setError(bool error);

    // Other
    /// @brief Cleans all the member of the instance.
    void                    clear();

private:
    Response(const Response &);
    Response &operator=(const Response &);

    QString                 version;
    int                     code;
    QString                 message;
    QString                 type;
    QVariantList            informations;
    QMap<QString, QString>  header;
    Content                 content;
    bool                    error;
};

#endif // RESPONSE_H
