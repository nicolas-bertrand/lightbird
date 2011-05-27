#ifndef CONFIGURATIONS_H
# define CONFIGURATIONS_H

# include <QMap>
# include <QMutex>
# include <QObject>
# include <QString>

# include "IConfiguration.h"

# include "Configuration.h"

/// @brief Configurations is a ThreadSafe class that manage all the Configuration.
/// The configurations loaded are cashed, and are deleted only when the server
/// configuration is deleted.
class Configurations : public QObject
{
    Q_OBJECT

public:
    /// @brief Used to create the server Configuration. Must be called before instance()
    /// @param path : The path to the server configuration. If empty, the path used
    /// will be DEFAULT_CONFIGURATION_DIRECTORY/DEFAULT_CONFIGURATION_FILE, and the
    /// alternative file is DEFAULT_CONFIGURATION_RESSOURCE.
    /// @param parent : The parent of the server configuration instance will be also
    /// the parent of all the other configurations.
    /// @return NULL is the loading fail.
    static Configuration    *server(const QString &path, QObject *parent = 0);
    /// @brief Returns an instance of the Configuration identified by the path
    /// to the configuration. If the configuration has not been already loaded,
    /// the document is parsed and stored before beeing returned.
    /// @param path : The path to the XML configuration file. If empty, the server
    /// configuration will be returned. This path can also be the id of a plugin.
    /// In this case, its configuration will be automatically recovered.
    /// @param alternative : The path of an alternative configuration file.
    /// It is used in the case where the file defined by path doesn't exists. The
    /// configuration pointed by path will be created using the alternative file.
    /// This is useful to use a default configuration from the resources.
    static Configuration    *instance(const QString &path = "", const QString &alternative = "");

private:
    static QMap<QString, Configuration *>   instances;     ///< The instances of the loaded configurations.
    static QMutex                           lockInstances; ///< Ensure that instances is thread safe.
};

#endif // CONFIGURATIONS_H
