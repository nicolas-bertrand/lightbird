#ifndef IRESOURCES_H
# define IRESOURCES_H

#include <QObject>

class QString;

namespace LightBird
{
    /// @brief This interface is called by the server to get the path of the resources of the
    /// plugins that implement it. This path must be unique to avoid conflicts with other resources.
    /// It allows the server to access to the plugin's resources. It can be used to create the
    /// configuration of the plugin on the file system, if it doesn't exists, or access to its queries.
    /// The plugin may not be loaded when this interface is called.
    class IResources
    {
    public:
        virtual                     ~IResources() {}

        /// @brief Returns the path of the resources of the plugin. This path must be unique to avoid
        /// conflicts with other resources.
        virtual QString             getResourcesPath() = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IResources, "cc.lightbird.IResources");

#endif // IRESOURCES_H
