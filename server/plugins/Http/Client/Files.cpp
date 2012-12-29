#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "IDatabase.h"

#include "Files.h"
#include "Plugin.h"
#include "TableFiles.h"

Files::Files()
{
}

Files::~Files()
{
}

void    Files::get(LightBird::IClient &client)
{
    LightBird::TableFiles file;
    QSqlQuery             query(Plugin::api().database().getDatabase());
    QVector<QVariantMap>  result;
    QJsonArray            array;

    query.prepare(Plugin::api().database().getQuery("HttpClient", "select_all_files"));
    if (!Plugin::api().database().query(query, result))
        return Plugin::response(client, 500, "Internal Server Error");
    QVectorIterator<QVariantMap> it(result);
    while (it.hasNext())
    {
        QJsonObject object;
        file.setId(it.peekNext()["id"].toString());
        QMapIterator<QString, QVariant> info(file.getInformations());
        while (info.hasNext())
        {
            object.insert(info.peekNext().key(), info.peekNext().value().toString());
            info.next();
        }
        object.insert("id", it.peekNext()["id"].toString());
        object.insert("name", it.peekNext()["name"].toString());
        object.insert("type", it.peekNext()["type"].toString());
        object.insert("id_directory", it.peekNext()["id_directory"].toString());
        object.insert("created", it.peekNext()["created"].toString());
        array.append(object);
        it.next();
    }
    client.getResponse().setType("application/json");
    (*client.getResponse().getContent().setStorage(LightBird::IContent::VARIANT).getVariant()) = QJsonDocument(array);
}
