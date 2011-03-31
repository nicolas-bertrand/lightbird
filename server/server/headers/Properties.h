#ifndef PROPERTIES_H
# define PROPERTIES_H

# include <QMap>
# include <QString>
# include <QVariant>

/// @brief Stores keys/values paires that represents properties. You can use it like this :
/// Properties().add("key1", "value1").add("key2", "value2");
class Properties
{
public:
    Properties();
    /// @brief Add directly a property at the construction.
    /// @see add
    Properties(const QString &key, const QVariant &value);
    Properties(const Properties &properties);
    Properties(const QMap<QString, QString> &properties);
    Properties                      &operator=(const QMap<QString, QString> &properties);

    /// @brief Add a property.
    /// @param key : The key of the property. A key may not be unique.
    /// @param value : The value of the property.
    /// @param addEmpty : If true, the property is added even if it is empty.
    /// @return A reference on the current object so that you can chain the calls.
    Properties                      &add(const QString &key, const QVariant &value, bool empty = true);
    /// @brief Add all the elements of the map to the properties.
    Properties                      &add(const QMap<QString, QVariant> &properties);
    /// @return The map of the properties.
    const QMap<QString, QString>    &toMap() const;

private:
    QMap<QString, QString>          properties;
};

#endif // PROPERTIES_H
