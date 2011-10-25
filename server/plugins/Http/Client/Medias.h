#ifndef MEDIAS_H
# define MEDIAS_H

# include <QMap>
# include <QMutex>
# include <QObject>

# include "IClient.h"

# include "Media.h"

class Medias : public QObject
{
    Q_OBJECT

public:
    static Medias   &getInstance(QObject *parent = NULL);

    void            start(LightBird::IClient &client, bool video);
    void            onFinish(LightBird::IClient &client);
    void            update(LightBird::IClient &client);
    void            stop(LightBird::IClient &client);
    void            disconnected(LightBird::IClient &client);

private:
    Medias(QObject *parent = NULL);
    ~Medias();

    QMap<QString, Media *>  medias;
    QStringList             stopList;
    QMutex                  mutex;
    static Medias           *instance;
};

#endif // MEDIAS_H
