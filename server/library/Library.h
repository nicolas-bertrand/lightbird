#ifndef LIBRARY_H
# define LIBRARY_H

# include <QObject>

# include "IConfiguration.h"
# include "IDatabase.h"
# include "ILogs.h"

# include "Export.h"

namespace LightBird
{
    /// @brief Allows the library to access some parts of the Api.
    class Library
    {
    public:
        static LightBird::IConfiguration &configuration();
        static LightBird::IDatabase      &database();
        static LightBird::ILogs          &log();

        LIB static void setConfiguration(LightBird::IConfiguration *configuration);
        LIB static void setDatabase(LightBird::IDatabase *database);
        LIB static void setLog(LightBird::ILogs *log);

    private:
        static LightBird::IConfiguration *_configuration;
        static LightBird::IDatabase      *_database;
        static LightBird::ILogs          *_log;
    };
}

#endif // LIBRARY_H
