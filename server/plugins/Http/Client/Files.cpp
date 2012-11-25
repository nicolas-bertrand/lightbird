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

void                Files::get(LightBird::IClient &client)
{
    LightBird::TableFiles file;
    QSqlQuery             query;
    QVector<QVariantMap>  result;
    QVariantMap           row;
    QVariantList          rows;

    query.prepare(Plugin::api().database().getQuery("HttpClient", "select_all_files"));
    if (!Plugin::api().database().query(query, result))
        return Plugin::response(client, 500, "Internal Server Error");
    QVectorIterator<QVariantMap> it(result);
    while (it.hasNext())
    {
        file.setId(it.peekNext()["id"].toString());
        row = file.getInformations();
        row.unite(it.next());
        rows.push_back(row);
    }
    client.getResponse().setType("application/json");
    (*client.getResponse().getContent().setStorage(LightBird::IContent::VARIANT).getVariant()) = rows;
}
