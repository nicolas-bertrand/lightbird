#ifndef LIGHTBIRD_PROPERTIES_H
# define LIGHTBIRD_PROPERTIES_H

# include <QMap>
# include <QString>
# include <QVariant>

# include "Export.h"

/// @brief Stores keys/values paires that represents properties.
/// @example Properties("key1", "value1").add("key2", "value2");
class LIB Properties
{
public:
    Properties();
    /// @brief Add directly a property at the construction.
    /// @see add
    Properties(const QString &key, const QVariant &value, bool empty = true);
    Properties(const QMap<QString, QString> &properties);
    Properties(const Properties &properties);
    Properties &operator=(const QMap<QString, QString> &properties);

    /// @brief Add a property.
    /// @param key : The key of the property. A key may not be unique.
    /// @param value : The value of the property.
    /// @param empty : If true the property is added even if the value is empty.
    /// @return A reference on the current object so that you can chain the calls.
    Properties                      &add(const QString &key, const QVariant &value, bool empty = true);
    /// @brief Add all the elements of the map to the properties.
    Properties                      &add(const QVariantMap &properties);
    /// @brief Converts the properties to a map.
    const QMap<QString, QString>    &toMap() const;
    /// @brief Clears the properties.
    void                            clear();

private:
    QMap<QString, QString>          properties;
};

#endif // LIGHTBIRD_PROPERTIES_H
