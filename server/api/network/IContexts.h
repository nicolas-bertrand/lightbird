#ifndef LIGHTBIRD_ICONTEXTS_H
# define LIGHTBIRD_ICONTEXTS_H

# include <QMap>
# include <QObject>
# include <QString>

namespace LightBird
{
    /// @brief Returns the contexts of the plugins.
    /// A context is a class that can implement any network interface, just
    /// like the main class of the plugin (LightBird::IPlugin). This allows
    /// to have multiple network classes that can be specialized based on a
    /// particular context (mode, protocol, port...).
    /// A context class must be instanciated as long as the plugin is loaded,
    /// and have to inherit from QObject.
    /// The contexts of a client can be changed using LightBird::IClient::getContexts.
    /// @see LightBird::IClient::getContexts
    class IContexts
    {
    public:
        virtual ~IContexts() {}

        /// @brief Returns the contexts of the plugin.
        /// The key represents the context name, and the value is the context
        /// itself, which must be instanciated as long as the plugin is loaded.
        virtual void    getContexts(QMap<QString, QObject *> &contexts) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IContexts, "cc.lightbird.IContexts")

#endif // LIGHTBIRD_ICONTEXTS_H
