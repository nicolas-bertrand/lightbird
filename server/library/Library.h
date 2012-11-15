#ifndef LIGHTBIRD_LIBRARY_H
# define LIGHTBIRD_LIBRARY_H

# include <QObject>

# include "IConfiguration.h"
# include "IDatabase.h"
# include "IExtensions.h"
# include "ILogs.h"

# include "Export.h"

class Preview;

namespace LightBird
{
    /// @brief Allows the library to access some parts of the Api, and to
    /// allocate various objects.
    class Library
    {
    public:
        static Library        *getInstance();
        // Api
        static IConfiguration &configuration();
        static IDatabase      &database();
        static IExtensions    &extension();
        static ILogs          &log();
        // Other
        static Preview        *getPreview();
        // These methods must only be used by the server
        LIB static void setConfiguration(IConfiguration *configuration);
        LIB static void setDatabase(IDatabase *database);
        LIB static void setExtension(IExtensions *extension);
        LIB static void setLog(ILogs *log);
        /// @brief Cleans the library.
        LIB static void shutdown();

    private:
        Library();
        ~Library();
        Library(const Library &);
        Library &operator=(const Library &);

        static Library *instance;
        // Api
        IConfiguration *_configuration;
        IDatabase      *_database;
        IExtensions    *_extension;
        ILogs          *_log;
        // Other
        Preview        *preview;
    };
}

#endif // LIGHTBIRD_LIBRARY_H
