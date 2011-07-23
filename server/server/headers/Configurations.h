#ifndef CONFIGURATIONS_H
# define CONFIGURATIONS_H

# include <QMap>
# include <QMutex>
# include <QObject>
# include <QString>

# include "Configuration.h"
# include "Initialize.h"

/// @brief Manages all the configurations of the server. The configurations loaded
/// are cashed, and deleted only when this object is destroyed.
class Configurations : public QObject,
                       public Initialize
{
    Q_OBJECT

public:
    /// @param path : The path to the server configuration. If empty, the path used
    /// will be DEFAULT_CONFIGURATION_DIRECTORY/DEFAULT_CONFIGURATION_FILE, and the
    /// alternative file is DEFAULT_CONFIGURATION_RESSOURCE.
    /// @param parent : The parent of the server configuration instance will be also
    /// the parent of all the other configurations.
    /// @return NULL is the loading fail.
    Configurations(const QString &path, QObject *parent = 0);
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
    Configuration        *getConfiguration(const QString &path = "", const QString &alternative = "");
    /// @brief Returns the instance of this class created by the Server.
    /// @see getConfiguration
    static Configuration *instance(const QString &path = "", const QString &alternative = "");

private:
    QMap<QString, Configuration *> instances; ///< The instances of the loaded configurations.
    QMutex                         mutex;     ///< Makes this class thread safe.
};

#endif // CONFIGURATIONS_H
