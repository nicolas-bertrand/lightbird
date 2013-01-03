#ifndef LIGHTBIRD_ICONTEXT_H
# define LIGHTBIRD_ICONTEXT_H

# include <QStringList>

namespace LightBird
{
    /// @brief Allows to get and set the informations of a context.
    /// The possible values of these informations are the same as the ones
    /// in the "contexts" node of the configuration of the plugins.
    /// They are all case insensitive.
    /// @see LightBird::IContexts
    /// @see LightBird::IClient::getContexts
    class IContext
    {
    public:
        virtual ~IContext() {}

        /// @brief Returns true if the contexts are equal.
        virtual bool    operator==(const LightBird::IContext &context) const = 0;
        /// @brief Returns true if the contexts are different.
        virtual bool    operator!=(const LightBird::IContext &context) const = 0;

        /// @brief Returns the name of the context, in lowercase.
        virtual QString getName() const = 0;

        /// @brief Returns the mode of the context, in lowercase.
        virtual QString getMode() const = 0;
        /// @brief Sets the mode of the context.
        /// @param mode : Can be "server", "client", or an empty string for both.
        virtual void    setMode(QString mode) = 0;

        /// @brief Returns the transport protocol of the context, in uppercase.
        virtual QString getTransport() const = 0;
        /// @brief Sets the transport protocol of the context.
        /// @param transport : Can be "TCP", "UDP", or an empty string for both.
        virtual void    setTransport(QString transport) = 0;

        /// @brief Returns the protocols of the context, in lowercase.
        virtual QStringList getProtocols() const = 0;
        /// @brief Adds several protocols to the context.
        /// @param protocols : The value "all" means all the protocols.
        virtual void    addProtocols(const QStringList &protocols) = 0;
        /// @brief Removes the protocols in the list from the context.
        virtual void    removeProtocols(const QStringList &protocols) = 0;

        /// @brief Returns the ports of the context.
        virtual QStringList getPorts() const = 0;
        /// @brief Adds several ports to the context.
        /// @param ports : The value "all" means all the ports.
        virtual void    addPorts(const QStringList &ports) = 0;
        /// @brief Removes the ports in the list from the context.
        virtual void    removePorts(const QStringList &ports) = 0;

        /// @brief Returns the methods of the context, in lowercase.
        virtual QStringList getMethods() const = 0;
        /// @brief Adds several methods to the context.
        virtual void    addMethods(const QStringList &methods) = 0;
        /// @brief Removes the methods in the list from the context.
        virtual void    removeMethods(const QStringList &methods) = 0;

        /// @brief Returns the types of the context, in lowercase.
        virtual QStringList getTypes() const = 0;
        /// @brief Adds several types to the context.
        virtual void    addTypes(const QStringList &types) = 0;
        /// @brief Removes the types in the list from the context.
        virtual void    removeTypes(const QStringList &types) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IContext, "cc.lightbird.IContext")

#endif // LIGHTBIRD_ICONTEXT_H
