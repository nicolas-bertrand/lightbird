#ifndef UPLOADS_H
# define UPLOADS_H

# include <QMap>
# include <QMutex>
# include <QObject>

# include "IClient.h"

class Uploads : public QObject
{
    Q_OBJECT

public:
    static Uploads  &getInstance(QObject *parent = NULL);

    struct Upload
    {
        qint64  size;
        qint64  progress;
        QString clientId;
    };

    void    start(LightBird::IClient &client);
    void    progress(LightBird::IClient &client);
    Upload  state(LightBird::IClient &client);
    void    stop(LightBird::IClient &client);
    void    disconnected(LightBird::IClient &client);

private:
    Uploads(QObject *parent = NULL);
    ~Uploads();

    QMap<QString, Upload>   uploads;
    QStringList             stopList;
    QMutex                  mutex;
    static Uploads          *instance;
};


#endif // UPLOADS_H
