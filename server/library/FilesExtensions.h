#ifndef LIGHTBIRD_FILESEXTENSIONS_H
# define LIGHTBIRD_FILESEXTENSIONS_H

# include <QVariantMap>

# include "IIdentify.h"

namespace LightBird
{
    /// @brief Allows to get the type and the MIME of a file based on its extension.
    class FilesExtensions
    {
    public:
        FilesExtensions();
        ~FilesExtensions();

        /// @brief Returns the type of the file based on its extension.
        /// @param fileName : The name of the file or just its extension.
        LightBird::IIdentify::Type getFileType(const QString &fileName);
        /// @brief Returns the MIME of the file based on its extension.
        /// The default MIME type is "application/octet-stream".
        /// @param fileName : The name of the file or just its extension.
        QString getFileMime(const QString &fileName);

    private:
        struct Data
        {
            LightBird::IIdentify::Type type;
            QString mime;
        };

        QMap<QString, Data> _extensions;
        QString _defaultMime;
    };
}

#endif // LIGHTBIRD_FILESEXTENSIONS_H
