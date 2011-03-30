#include "Properties.h"

Properties::Properties()
{
}

Properties::Properties(const QString &key, const QVariant &value)
{
    this->add(key, value.toString());
}

Properties::Properties(const Properties &properties)
{
    this->properties = properties.properties;
}

Properties::Properties(const QMap<QString, QString> &properties)
{
    this->properties = properties;
}

Properties  &Properties::operator=(const QMap<QString, QString> &properties)
{
    this->properties = properties;
    return (*this);
}

Properties  &Properties::add(const QString &key, const QVariant &value, bool empty)
{
    if (empty == true || !value.toString().isEmpty())
        this->properties.insertMulti(key, value.toString());
    return (*this);
}

Properties  &Properties::add(const QMap<QString, QVariant> &properties)
{
    QMapIterator<QString, QVariant> it(properties);

    while (it.hasNext())
    {
        it.next();
        this->properties.insertMulti(it.key(), it.value().toString());
    }
    return (*this);
}

const QMap<QString, QString>    &Properties::toMap() const
{
    return (this->properties);
}
