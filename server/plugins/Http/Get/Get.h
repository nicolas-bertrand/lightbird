#ifndef GET_H
# define GET_H

# include <QObject>

# include "IPlugin.h"
# include "IDoExecution.h"

class Get : public QObject,
            public LightBird::IPlugin,
            public LightBird::IDoExecution
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IDoExecution)

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

private:
    LightBird::IApi *api;
};

#endif // GET_H
