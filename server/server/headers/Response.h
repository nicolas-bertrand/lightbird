#ifndef RESPONSE_H
# define RESPONSE_H

# include <QObject>

# include "Content.h"

# include "IResponse.h"

/// @brief The server implementation of LightBird::IResponse.
class Response : public QObject,
                 public LightBird::IResponse
{
    Q_OBJECT

public:
    Response(QObject *parent = NULL);
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
    bool                    isError() const;
    void                    setError(bool error = true);

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
