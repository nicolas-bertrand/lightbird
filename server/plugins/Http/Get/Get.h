#ifndef GET_H
# define GET_H

# include <QObject>
# include <QWaitCondition>
# include <QMutex>

# include "IDoExecution.h"
# include "IDoSend.h"
# include "IOnUnserialize.h"
# include "IPlugin.h"

class Get : public QObject,
            public LightBird::IPlugin,
            public LightBird::IDoExecution,
            public LightBird::IDoSend,
            public LightBird::IOnUnserialize
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IDoExecution
                 LightBird::IDoSend
                 LightBird::IOnUnserialize)

public:
    Get();
    ~Get();

    // IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // Execution
    bool    doExecution(LightBird::IClient &client);
    bool    doSend(LightBird::IClient &client);
    void    onUnserialize(LightBird::IClient &client, LightBird::IOnUnserialize::Unserialize type);

private:
    LightBird::IApi *api;
    QMutex          mutex;
    QWaitCondition  wait;
    QString         string;
};

#endif // GET_H
