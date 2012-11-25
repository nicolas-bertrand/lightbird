#ifndef COMMANDS_H
# define COMMANDS_H

# include "IApi.h"
# include "IClient.h"

class Plugin;

class Commands
{
public:
    Commands();
    ~Commands();

    void    execute(LightBird::IClient &client, QString command);

private:
    Commands(const Commands &);
    Commands &operator=(const Commands &);

    void    _audio(LightBird::IClient &client, const QString &command);
    void    _disconnect(LightBird::IClient &client, const QString &command);
    void    _identify(LightBird::IClient &client, const QString &command);
    void    _preview(LightBird::IClient &client, const QString &command);
    void    _select(LightBird::IClient &client, const QString &command);
    void    _files(LightBird::IClient &client, const QString &command);
    void    _uploads(LightBird::IClient &client, const QString &command);
    void    _stopStream(LightBird::IClient &client, const QString &command);
    void    _video(LightBird::IClient &client, const QString &command);
    void    _deleteFile(LightBird::IClient &client, const QString &command);

    typedef void (Commands::*command)(LightBird::IClient &, const QString &);

    QMap<QString, command> commands;
};

#endif // COMMANDS_H
