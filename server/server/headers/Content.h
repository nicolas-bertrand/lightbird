#ifndef CONTENT_H
# define CONTENT_H

# include <QByteArray>
# include <QVariant>
# include <QFile>
# include <QTemporaryFile>

# include "IContent.h"

/// @brief Server's implementation of IContent.
class Content : public QObject,
                public LightBird::IContent
{
    Q_OBJECT

public:
    Content(QObject *parent = 0);
    ~Content();

    LightBird::IContent::Storage getStorage() const;
    void            setStorage(LightBird::IContent::Storage storage, const QString &fileName = "");
    QByteArray      getContent(quint64 size = 0);
    void            setContent(const QByteArray &content, bool append = true);
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
    QByteArray                   *byteArray;
    QVariant                     *variant;
    QFile                        *file;
    QTemporaryFile               *temporaryFile;
    quint64                      seek;
};

#endif // CONTENT_H
