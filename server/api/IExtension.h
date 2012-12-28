#ifndef LIGHTBIRD_IEXTENSION_H
# define LIGHTBIRD_IEXTENSION_H

# include <QString>
# include <QStringList>

namespace LightBird
{
    /// @brief Plugins that implements this interface can be used as extensions
    /// by other plugins.
    class IExtension
    {
    public:
        virtual ~IExtension() {}

        /// @brief Returns the names of the extensions implemented by this plugin.
        virtual QStringList getExtensionsNames() = 0;
        /// @brief Returns a pointer to an implemented extension, or NULL.
        /// The caller has to cast the returned pointer into the correct extension
        /// in order to use it. The extension instance returned here must not be destroyed
        /// before the call to releaseExtension().
        /// @param name : The name of the extension to return.
        /// @return A pointer to the extension, or NULL if the plugin doesn't implements it.
        virtual void        *getExtension(const QString &name) = 0;
        /// @brief Release an extension obtained by getExtension(). After this call, the
        /// plugin is free to delete the extension immediatly, wait its unloading, or reuse it.
        /// @param name : The name of the extension.
        /// @param extension : The pointer returned by getExtension().
        virtual void        releaseExtension(const QString &name, void *extension) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IExtension, "cc.lightbird.IExtension")

#endif // LIGHTBIRD_IEXTENSION_H
