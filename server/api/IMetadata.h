#ifndef IMETADATA_H
# define IMETADATA_H

# include <QString>
# include <QStringList>
# include <QMap>

namespace LightBird
{
    /// @brief Stores information about a plugin.
    struct                     IMetadata
    {
        QString                name;        ///< The real name of the plugin.
        QString                brief;       ///< A brief description of its features.
        QString                description; ///< A more detailled description.
        QString                autor;       ///< The autor of the plugin.
        QString                site;        ///< The site that support the plugin.
        QString                email;       ///< Email of the autor.
        QString                version;     ///< The version of the plugin.
        QString                licence;     ///< The licence under which the plugin is distributed.
        QMap<QString, QString> other;       ///< Other relevant information.
    };
}

#endif // IMETADATA_H
