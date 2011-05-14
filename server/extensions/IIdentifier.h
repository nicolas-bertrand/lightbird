#ifndef IIDENTIFIER_H
# define IIDENTIFIER_H

# include <QMap>
# include <QString>
# include <QVariant>

# include "IIdentify.h"

namespace LightBird
{
    /// @brief Allows plugins to get information on a file.
    ///
    /// This extension is implemented by a plugin that will calls the IIdentify interface
    /// of each plugins that implements it. Then it will try to mix all the data gathered,
    /// in order to return the final list of information on the file, and its type.
    class IIdentifier
    {
    public:
        virtual ~IIdentifier() {}

        /// @brief Get information on a file.
        /// @param file : The name of the file that will be probed.
        /// @return The information on the file, and its type.
        virtual LightBird::IIdentify::Information   identify(const QString &file) = 0;
    };
}

#endif // IIDENTIFIER_H
