#ifndef EXECUTE_H
# define EXECUTE_H

# include "IApi.h"
# include "IClient.h"
# include "IRequest.h"
# include "IResponse.h"

# define BUFFER_COPY_SIZE 1000000

class Execute
{
public:
    Execute(LightBird::IApi &api, LightBird::IClient &client, const QString &command);
    ~Execute();

private:
    Execute(const Execute &);
    Execute &operator=(const Execute &);

    typedef void         (Execute::*func)();

    void                 _audio();
    void                 _disconnect();
    void                 _identify();
    void                 _preview();
    void                 _select();
    void                 _uploads();
    void                 _uploadsProgress();
    void                 _stopUpload();
    void                 _stopStream();
    void                 _video();
    void                 _deleteFile();

    LightBird::IApi      &api;
    LightBird::IClient   &client;
    LightBird::IRequest  &request;
    LightBird::IResponse &response;
    QMap<QString, func>  commands;
};

#endif // EXECUTE_H
