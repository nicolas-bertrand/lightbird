#ifndef LIGHTBIRD_IIDENTIFY_H
# define LIGHTBIRD_IIDENTIFY_H

# include <QMap>
# include <QString>
# include <QVariant>

namespace LightBird
{
    /// @brief Allows plugins to get information on a file.
    class IIdentify
    {
    public:
        virtual ~IIdentify() {}

        /// @brief List the file types available.
        enum Type
        {
            AUDIO,
            DOCUMENT,
            IMAGE,
            OTHER,
            VIDEO
        };

        /// @brief Stores the information that has been collected on the file.
        struct Information
        {
            Type        type; ///< The type of the file.
            QVariantMap data; ///< More data on the file.
        };

        /// @brief Gets information on a file.
        /// @param file : The path of the file that will be probed.
        /// @param information : If true is returned, this parameter is filled
        /// with the information on the file.
        /// @return True if the extension has been able to get information on the file.
        virtual bool    identify(const QString &file, LightBird::IIdentify::Information &information) = 0;
    };
}

#endif // LIGHTBIRD_IIDENTIFY_H
