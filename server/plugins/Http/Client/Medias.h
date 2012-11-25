#ifndef MEDIAS_H
# define MEDIAS_H

# include <QMap>
# include <QMutex>

# include "IApi.h"
# include "IClient.h"
# include "IIdentify.h"

# include "Media.h"

class Medias
{
public:
    Medias();
    ~Medias();

    enum Type
    {
        AUDIO,
        VIDEO
    };

    void    start(LightBird::IClient &client, Medias::Type type = Medias::VIDEO);
    void    onFinish(LightBird::IClient &client);
    void    update(LightBird::IClient &client);
    void    stop(LightBird::IClient &client);
    void    disconnected(LightBird::IClient &client);

private:
    Medias(const Medias &);
    Medias &operator=(const Medias &);

    QMap<QString, Media *> medias;
    QStringList            stopList;
    QMutex                 mutex;
};

#endif // MEDIAS_H
