#ifndef LIGHTBIRD_IMETADATA_H
# define LIGHTBIRD_IMETADATA_H

# include <QString>
# include <QVariantMap>

namespace LightBird
{
    /// @brief Stores informations about a plugin.
    struct                     IMetadata
    {
        QString     name;         ///< The full name of the plugin.
        QString     brief;        ///< A brief description of its features.
        QString     description;  ///< A more detailled description.
        QString     autor;        ///< The autor of the plugin.
        QString     site;         ///< The site that supports the plugin.
        QString     email;        ///< Email of the autor.
        QString     version;      ///< The version of the plugin.
        QString     licence;      ///< The licence under which the plugin is distributed.
        QStringList dependencies; ///< List the dependances of the plugin (i.e. third-party libraries).
        QVariantMap other;        ///< Other relevant information.
    };
}

#endif // LIGHTBIRD_IMETADATA_H
