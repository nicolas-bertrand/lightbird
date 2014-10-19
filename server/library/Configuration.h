#ifndef LIGHTBIRD_CONFIGURATION_H
# define LIGHTBIRD_CONFIGURATION_H

# include <QObject>
# include <QStringList>

# include "ILogs.h"
# include "IConfiguration.h"
# include "Export.h"

namespace LightBird
{
    class Library;

    /// @brief Provides a fast way to access the general configuration of the server.
    class Configuration
    {
    public:
        QString name;
        QString pluginsPath;
        QString QtPluginsPath;
        QString filesPath;
        QString temporaryPath;
        bool cleanTemporaryPath;
        QString languagesPath;
        QString language;
        uint threadsNumber;
        qint64 hashSizeLimit;

        struct Database
        {
            QString name;
            QString file;
            QString path;
            QString resource;
            QString type;
            QString password;
            QString user;
            QString host;
            ushort port;
        };
        Database database;

        struct Permissions
        {
            bool activate;
            bool default_;
            bool inheritance;
            bool ownerInheritance;
            bool groupInheritance;
        };
        Permissions permissions;

        struct Log
        {
            ILogs::Level level;
            bool display;
            QString file;
            QString path;
            uint maxNbOfFile;
            quint64 maxSize;
            uint expires;
        };
        Log log;

        struct Preview
        {
            bool cacheEnabled;
            QString cachePath;
            quint64 cacheSizeLimit;
        };
        Preview preview;

        /// @brief Updates the values of the configurations.
        LIB static inline void update() { _instance->_update(); }
        /// @brief Returns the instance of the configuration.
        LIB static inline const Configuration &get() { return *_instance; }
        /// @brief Sets a value.
        LIB static inline void set(const QString &nodeName, const QString &nodeValue) { _instance->_set(nodeName, nodeValue); }

    private:
        Configuration(LightBird::IConfiguration *configuration);
        ~Configuration();

        void _update();
        void _set(const QString &nodeName, const QString &nodeValue);

        LightBird::IConfiguration &_c;
        static Configuration *_instance;

        friend class Library;
    };
}

#endif // LIGHTBIRD_CONFIGURATION_H
