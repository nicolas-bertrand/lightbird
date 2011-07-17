#ifndef APICONFIGURATION_H
# define APICONFIGURATION_H

# include <QObject>

# include "Configuration.h"

/// @brief The server implementation of the IConfiguration interface.
///
/// The configuration of the plugins are stored in the "configurations" node
/// of the configuration of the server. To simplify their access, this class
/// acts as if these configuration were separate files, by inheriting Configuration.
/// One instance manages one plugin configuration.
class ApiConfiguration : public Configuration
{
    Q_OBJECT

public:
    /// @param id : The id of the plugin for which the object has been created.
    ApiConfiguration(const QString &id);
    ~ApiConfiguration();

    QString         getPath();
    QString         get(const QString &nodeName);
    unsigned        count(const QString &nodeName);
    void            set(const QString &nodeName, const QString &nodeValue);
    bool            remove(const QString &nodeName);
    QDomElement     readDom();
    QDomElement     writeDom();
    void            release();
    bool            save();

private:
    ApiConfiguration();
    ApiConfiguration(const ApiConfiguration &);
    ApiConfiguration &operator=(const ApiConfiguration &);

    /// @brief Returns the element that contains the configuration of the plugin.
    /// @param dom : The root element of the configuration of the server.
    QDomElement     _findConfiguration(QDomElement root);

    QString         id;             ///< The id of the plugin for which the object has been created.
    Configuration   &configuration; ///< The configuration of the server.
};

#endif // APICONFIGURATION_H
