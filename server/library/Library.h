#ifndef LIGHTBIRD_LIBRARY_H
# define LIGHTBIRD_LIBRARY_H

# include <QObject>

# include "IConfiguration.h"
# include "IDatabase.h"
# include "IExtensions.h"
# include "IImage.h"
# include "ILogs.h"

# include "Export.h"

namespace LightBird
{
    class Configuration;
    class FilesExtensions;
    class Identify;
    class Preview;

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
        static LightBird::Identify *getIdentify();
        static LightBird::Preview *getPreview();
        static LightBird::FilesExtensions *getFilesExtensions();
        static QHash<LightBird::IImage::Format, QString> &getImageExtensions();
        static QHash<QString, LightBird::IImage::Format> &getImageFormats();

        // These methods must only be used by the server
        /// @brief Initializes the library.
        LIB static void        initialize();
        LIB static void        setConfiguration(IConfiguration *configuration);
        LIB static void        setDatabase(IDatabase *database); ///< Takes ownership of database.
        LIB static void        setExtension(IExtensions *extension);
        LIB static void        setLog(ILogs *log); ///< Takes ownership of log.
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
        LightBird::Identify *_identify;
        LightBird::Preview *_preview;
        LightBird::FilesExtensions *_filesExtensions;
        QHash<LightBird::IImage::Format, QString> _imageExtensions;
        QHash<QString, LightBird::IImage::Format> _imageFormats;
    };
}

#endif // LIGHTBIRD_LIBRARY_H
