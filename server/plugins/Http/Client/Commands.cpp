#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>

#include "Audio.h"
#include "Commands.h"
#include "LightBird.h"
#include "Medias.h"
#include "Plugin.h"
#include "Preview.h"
#include "Uploads.h"

Commands::Commands()
{
    this->commands["audio"] = &Commands::_audio;
    this->commands["disconnect"] = &Commands::_disconnect;
    this->commands["identify"] = &Commands::_identify;
    this->commands["preview"] = &Commands::_preview;
    this->commands["select"] = &Commands::_select;
    this->commands["files"] = &Commands::_files;
    this->commands["uploads"] = &Commands::_uploads;
    this->commands["video"] = &Commands::_video;
    this->commands["delete_file"] = &Commands::_deleteFile;
}

Commands::~Commands()
{
}

void    Commands::execute(LightBird::IClient &client, QString command)
{
    QString parameter;

    // Get the command
    if (command.contains("."))
        command = command.left(command.indexOf("."));
    // The client is not connected and it doesn't tries to identify
    if ((!client.getAccount().exists() || QUrlQuery(client.getRequest().getUri()).queryItemValue("identifiant").isEmpty()) && command != "identify")
    {
        Plugin::response(client, 403, "Forbidden");
        return ;
    }
    // Execute the command
    if (command.contains('/'))
    {
        command = (parameter = command).left(command.indexOf('/'));
        parameter = parameter.right(parameter.size() - command.size() - 1);
    }
    if (this->commands.contains(command))
        (this->*(this->commands[command]))(client, parameter);
    // The command is unknow
    else
        Plugin::response(client, 404, "Not Found");
}

void    Commands::_audio(LightBird::IClient &client, const QString &parameter)
{
    if (parameter.isEmpty())
        Plugin::medias().start(client, Medias::AUDIO);
    else if (parameter == "stop")
        Plugin::medias().stop(client);
}

void    Commands::_disconnect(LightBird::IClient &client, const QString &)
{
    // The session is destroyed and all the clients associated to it are disconnected
    Plugin::api().sessions().destroy(client.getSessions().first(), true);
    // Tells onDisconnect that we want the final response to be sent before the disconnection of this client
    client.getInformations().insert("delay_disconnection", true);
}

void    Commands::_identify(LightBird::IClient &client, const QString &)
{
    LightBird::Session      session;
    QString                 salt;
    QString                 name;
    QString                 id;
    QString                 json = "{ \"sessionId\" : \"%1\", \"salt\" : \"%2\" }";
    QString                 identifiant;
    LightBird::IDatabase    &database = Plugin::api().database();
    QSqlQuery               query(database.getDatabase());
    QVector<QVariantMap>    result;
    int                     i = 0;
    int                     s;
    QString                 sessionId;
    QUrlQuery               urlQuery(client.getRequest().getUri());

    // If too many identification failed attempts has been done
    if (!Plugin::instance().identificationAllowed(client))
        return Plugin::response(client, 403, "Forbidden");
    // The client is asking the id of the account and a session id, in order to
    // generate the identifiant : SHA-256(name + SHA-256(password + accountId) + sessionId)
    if (!(name = urlQuery.queryItemValue("name")).isEmpty() &&
        (salt = urlQuery.queryItemValue("salt")).size() >= 32)
    {
        // Searches the id of the account using the salt and its name
        query.prepare(database.getQuery("HttpClient", "select_all_accounts"));
        if (database.query(query, result))
            for (i = 0, s = result.size(); i < s && id.isEmpty(); ++i)
                if (name == LightBird::sha256(result[i]["name"].toByteArray() + salt.toLatin1()))
                {
                    id = result[i]["id"].toString();
                    break;
                }
        // The account has been found
        if (!id.isEmpty())
        {
            // Create a new session. The client has 30 seconds to generate the identifiant.
            session = Plugin::api().sessions().create(QDateTime::currentDateTime().addSecs(30), id, QStringList() << client.getId());
            // Compute the identifiant of the client
            session->setInformation("identifiant", LightBird::sha256(result[i]["name"].toByteArray() + result[i]["password"].toByteArray() + session->getId().toLatin1()));
            // Return a json that contains the session id and the id of the account, which is the salt of the password
            json = json.arg(session->getId(), id);
            client.getResponse().getContent().setData(json.toLatin1());
        }
        // Otherwise we return a fake salt and session id (the user shouldn't know that the name does not exist)
        else
        {
            json = json.arg(LightBird::createUuid(), LightBird::createUuid());
            client.getResponse().getContent().setData(json.toLatin1());
            Plugin::instance().identificationFailed(client);
        }
    }
    // Tries to identify the user using the identifiant generated by the client
    else if (!(identifiant = urlQuery.queryItemValue("identifiant")).isEmpty() &&
             !(sessionId = urlQuery.queryItemValue("sessionId")).isEmpty() &&
             !(session = Plugin::api().sessions().getSession(sessionId)).isNull() &&
             !client.getResponse().isError() &&
             client.getAccount().setId(session->getAccount()) &&
             client.getAccount().isActive())
    {
        session->setAccount(client.getAccount().getId());
        session->setExpiration(QDateTime::currentDateTime().addMonths(1));
    }
    // The identification failed
    else
    {
        Plugin::response(client, 403, "Forbidden");
        client.getAccount().clear();
        if (!session.isNull())
            session->destroy();
    }
}

void    Commands::_preview(LightBird::IClient &client, const QString &)
{
    Preview preview(client);
    preview.generate();
    client.getResponse().getHeader().insert("cache-control", "private");
}

void    Commands::_select(LightBird::IClient &client, const QString &)
{
    LightBird::TableFiles file;
    QSqlQuery             query(Plugin::api().database().getDatabase());
    QVector<QVariantMap>  result;
    int                   s = 0;
    QVariantMap           row;
    QList<QVariant>       rows;

    query.prepare(Plugin::api().database().getQuery("HttpClient", "select_all_files"));
    if (!Plugin::api().database().query(query, result))
        return Plugin::response(client, 500, "Internal Server Error");
    s = s;
    for (int i = 0, s = result.size(); i < s; ++i)
    {
        file.setId(result[i]["id"].toString());
        row = file.getInformations();
        row.unite(result[i]);
        rows.push_back(row);
    }
    client.getResponse().getContent().setStorage(LightBird::IContent::VARIANT);
    *client.getResponse().getContent().getVariant() = rows;
    client.getResponse().setType("application/json");
}

void    Commands::_files(LightBird::IClient &client, const QString &parameter)
{
    if (parameter == "get")
        Plugin::files().get(client);
    else if (parameter == "update")
        Plugin::files().update(client);
}

void    Commands::_uploads(LightBird::IClient &client, const QString &parameter)
{
    if (parameter == "cancel")
        Plugin::uploads().cancel(client);
}

void    Commands::_video(LightBird::IClient &client, const QString &parameter)
{
    if (parameter.isEmpty())
        Plugin::medias().start(client, Medias::VIDEO);
    else if (parameter == "stop")
        Plugin::medias().stop(client);
}

void    Commands::_deleteFile(LightBird::IClient &client, const QString &)
{
    LightBird::TableFiles file(QUrlQuery(client.getRequest().getUri()).queryItemValue("id"));

    if (file.exists())
    {
        if (file.isAllowed(client.getAccount().getId(), "delete"))
        {
            QString path = file.getFullPath();
            file.remove();
            QFile::remove(path);
        }
        else
            Plugin::response(client, 403, "Forbidden");
    }
    else
        Plugin::response(client, 404, "Not Found");
}
