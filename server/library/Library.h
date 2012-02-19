#ifndef LIBRARY_H
# define LIBRARY_H

# include <QObject>

# include "ILogs.h"

# include "Export.h"

namespace LightBird
{
    /// @brief Allows the library to access some parts of the Api.
    class Library
    {
    public:
        static LightBird::ILogs &log();

        LIB static void         setLogs(LightBird::ILogs *logs);

    private:
        static LightBird::ILogs *logs;
    };
}

#endif // LIBRARY_H
