#ifndef LIGHTBIRD_IIDENTIFY_H
# define LIGHTBIRD_IIDENTIFY_H

# include <QMap>
# include <QString>
# include <QVariant>
# include <QList>

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
            OTHER = 0,
            AUDIO,
            DOCUMENT,
            IMAGE,
            VIDEO
        };

        /// @brief Stores the information that has been collected on the file.
        struct Information
        {
            Type        type; ///< The type of the file.
            QVariantMap data; ///< More data on the file.
        };

        /// @brief Returns the list of the types the extension can identify.
        inline const QList<LightBird::IIdentify::Type> &types() { return _types; }

        /// @brief Gets information on a file.
        /// @param file : The path of the file that will be probed.
        /// @param information : If true is returned, this parameter is filled
        /// with the information on the file.
        /// @return True if the extension has been able to get information on the file.
        virtual bool    identify(const QString &file, LightBird::IIdentify::Information &information) = 0;

    protected:
        ///< The list of the types the extension can identify.
        QList<LightBird::IIdentify::Type> _types;
    };
}

Q_DECLARE_INTERFACE(LightBird::IIdentify, "cc.lightbird.IIdentify")

#endif // LIGHTBIRD_IIDENTIFY_H
