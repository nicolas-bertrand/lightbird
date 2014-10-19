#ifndef LIGHTBIRD_LIBRARY_H
# define LIGHTBIRD_LIBRARY_H

# include <QObject>

# include "IConfiguration.h"
# include "IDatabase.h"
# include "IExtensions.h"
# include "IImage.h"
# include "ILogs.h"

# include "Export.h"

class Identify;
class Preview;
class FilesExtensions;

namespace LightBird
{
    class Configuration;

    /// @brief Allows the library to access some parts of the Api, and to
    /// allocate various objects.
    class Library
    {
    public:
        static Library         *getInstance();
        // Api
        static IConfiguration  &configuration();
        static IDatabase       &database();
        static IExtensions     &extension();
        static ILogs           &log();
        // Other
        static Identify        *getIdentify();
        static Preview         *getPreview();
        static FilesExtensions *getFilesExtensions();
        static QHash<LightBird::IImage::Format, QString> &getImageExtensions();

        // These methods must only be used by the server
        /// @brief Initializes the library.
        LIB static void        initialize();
        LIB static void        setConfiguration(IConfiguration *configuration);
        LIB static void        setDatabase(IDatabase *database);
        LIB static void        setExtension(IExtensions *extension);
        LIB static void        setLog(ILogs *log);
        /// @brief Cleans the library.
        LIB static void        shutdown();

    private:
        Library();
        ~Library();
        Library(const Library &);
        Library &operator=(const Library &);

        static Library  *_instance;
        // Api
        IConfiguration  *_configuration;
        IDatabase       *_database;
        IExtensions     *_extension;
        ILogs           *_log;
        // Other
        Configuration   *_c;
        Identify        *_identify;
        Preview         *_preview;
        FilesExtensions *_filesExtensions;
        QHash<LightBird::IImage::Format, QString> _imageExtensions;
    };
}

#endif // LIGHTBIRD_LIBRARY_H
