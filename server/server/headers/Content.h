#ifndef CONTENT_H
# define CONTENT_H

# include <QObject>

# include "IContent.h"

/// @brief Server implementation of LightBird::IContent.
class Content : public QObject,
                public LightBird::IContent
{
    Q_OBJECT

public:
    Content(QObject *parent = NULL);
    ~Content();

    LightBird::IContent::Storage getStorage() const;
    LightBird::IContent &setStorage(LightBird::IContent::Storage storage, const QString &fileName = "");
    QByteArray      getData(quint64 size = 0);
    void            setData(const QByteArray &data, bool append = true);
    QByteArray      *getByteArray();
    QVariant        *getVariant();
    QFile           *getFile();
    QTemporaryFile  *getTemporaryFile();
    qint64          size() const;
    qint64          getSeek() const;
    void            setSeek(qint64 position);
    void            clear();

private:
    Content(const Content &);
    Content &operator=(const Content &);

    LightBird::IContent::Storage storage;
    QByteArray      *byteArray;
    QVariant        *variant;
    QFile           *file;
    QTemporaryFile  *temporaryFile;
    quint64         seek;
    bool            validFile;
};

#endif // CONTENT_H
