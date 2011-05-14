#ifndef IMIME_H
# define IMIME_H

# include <QString>

namespace LightBird
{
    /// @brief Get the MIME type of a file from its extension.
    class IMime
    {
    public:
        virtual ~IMime() {}

        /// @brief Return the MIME type of a file, based on its extension.
        /// The default MIME type is "application/octet-stream".
        /// @param file : The name of the file.
        /// @return The MIME type of the file.
        virtual QString getMime(const QString &file) = 0;
    };
}

#endif // IMIME_H
