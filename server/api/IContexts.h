#ifndef LIGHTBIRD_ICONTEXTS_H
# define LIGHTBIRD_ICONTEXTS_H

# include <QMap>
# include <QObject>
# include <QStringList>

# include "IContext.h"

namespace LightBird
{
    /// @brief Manages the contexts of the current plugin.
    /// The contexts of a plugin are loaded from the "contexts" node of its
    /// configuration, just after IPlugin::onLoad.
    /// @see LightBird::IContext
    /// @see LightBird::IClient::getContexts
    class IContexts
    {
    public:
        virtual ~IContexts() {}

        /// @brief Declares a context instance.
        /// An instance is a class that can implement any network interface,
        /// just like the main class of the plugin (LightBird::IPlugin). After
        /// its declaration, one can associate multiple contexts to the instance.
        /// This allows to have multiple network classes that can be specialized
        /// based on a particular context (mode, protocol, port...).
        /// @warning Once an instance has been declared, it is not possible to
        /// remove it and it must stay instanciated as long as the plugin is loaded.
        /// @warning It is the responsability of the plugin to delete its
        /// declared instances in LightBird::IPlugin::onUnload.
        /// @param name : The name is used to associate an instance with a context.
        /// @param instance : The instance must implement QObject, and must be
        /// instanciated as long as the plugin is loaded.
        /// @return False if the name is already used by an instance.
        virtual bool    declareInstance(QString name, QObject *instance) = 0;

        /// @brief Loads the contexts from the "contexts" node of the plugin
        /// configuration. This is normally done automatically just after
        /// IPlugin::onLoad, but if a plugin needs its contexts in onLoad,
        /// loadContextsFromConfiguration can be called, after declareInstance.
        virtual void    loadContextsFromConfiguration() = 0;

        /// @brief Returns the contexts of the plugin.
        /// @param names : The list of the contexts to return.
        /// Several contexts can share the same name (the same instance).
        /// An empty list will returns all the contexts.
        /// An empty string in the list will return the default contexts.
        /// @warning The pointers returned by this method are only valid in the
        /// method that called it.
        virtual QMultiMap<QString, LightBird::IContext *> get(QStringList names = QStringList()) = 0;
        /// @see IContexts::get
        virtual QMultiMap<QString, LightBird::IContext *> get(QString name) = 0;

        /// @brief Adds a context and returns a pointer to it.
        /// @param name : The name of the new context, which must match an
        /// instance declared by the plugin in IContexts::declareInstance.
        /// An empty name is the default context.
        /// @return The new context, or NULL if the name does not exist.
        /// @warning The pointer returned by this method is only valid in the
        /// method that called it.
        /// @see IContexts::getContexts
        virtual LightBird::IContext *add(const QString &name) = 0;

        /// @brief Clones the given context, and sets a new name to the result.
        /// @param context : The context to clone.
        /// @param newName : The name of the new context, which must match an
        /// instance declared by the plugin in IContexts::declareInstance.
        /// @return The new context, or NULL if the newName does not exist.
        /// @warning The pointer returned by this method is only valid in the
        /// method that called it.
        virtual LightBird::IContext *clone(LightBird::IContext *context, const QString &newName) = 0;

        /// @brief Removes a context.
        /// @warning The context pointer will no longer be valid after this call.
        virtual void    remove(LightBird::IContext *context) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IContexts, "cc.lightbird.IContexts")

#endif // LIGHTBIRD_ICONTEXTS_H
