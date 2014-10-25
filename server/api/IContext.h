#ifndef LIGHTBIRD_ICONTEXT_H
# define LIGHTBIRD_ICONTEXT_H

# include <QStringList>

# include "IClient.h"
# include "INetwork.h"

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
        /// @brief Sets the mode of the context.
        virtual void    setMode(LightBird::IClient::Mode) = 0;

        /// @brief Returns the transport protocol of the context, in uppercase.
        virtual QString getTransport() const = 0;
        /// @brief Sets the transport protocol of the context.
        /// @param transport : Can be "TCP", "UDP", or an empty string for both.
        virtual void    setTransport(QString transport) = 0;
        /// @brief Sets the transport protocol of the context.
        virtual void    setTransport(LightBird::INetwork::Transport transport) = 0;

        /// @brief Returns the protocols of the context, in lowercase.
        virtual QStringList getProtocols() const = 0;
        /// @brief Adds several protocols to the context.
        /// @param protocols : The value "all" means all the protocols.
        virtual void    addProtocols(const QStringList &protocols) = 0;
        /// @see addProtocols
        inline  void    addProtocol(const QString &protocol) { addProtocols(QStringList(protocol)); }
        /// @brief Removes the protocols in the list from the context.
        virtual void    removeProtocols(const QStringList &protocols) = 0;
        /// @see removeProtocols
        inline  void    removeProtocol(const QString &protocol) { removeProtocols(QStringList(protocol)); }

        /// @brief Returns the ports of the context.
        virtual QStringList getPorts() const = 0;
        /// @brief Adds several ports to the context.
        /// @param ports : The value "all" means all the ports.
        virtual void    addPorts(const QStringList &ports) = 0;
        /// @brief Adds the ports in the string. The ports can be separated by
        /// any character excluding '-', which is used to represent a range of
        /// ports between two numbers. The value "all" is also understood.
        virtual void    addPorts(const QString &ports) = 0;
        /// @brief Adds the port in parameter, which must not be 0.
        virtual void    addPort(quint16 port) = 0;
        /// @brief Removes the ports in the list from the context.
        virtual void    removePorts(const QStringList &ports) = 0;
        /// @brief Removes the port in parameter.
        virtual void    removePort(quint16 port) = 0;

        /// @brief Returns the methods of the context, in lowercase.
        virtual QStringList getMethods() const = 0;
        /// @brief Adds several methods to the context.
        virtual void    addMethods(const QStringList &methods) = 0;
        /// @see addMethods
        virtual void    addMethod(const QString &method) { addMethods(QStringList(method)); }
        /// @brief Removes the methods in the list from the context.
        virtual void    removeMethods(const QStringList &methods) = 0;
        /// @see removeMethods
        inline  void    removeMethod(const QString &method) { removeMethods(QStringList(method)); }

        /// @brief Returns the types of the context, in lowercase.
        virtual QStringList getTypes() const = 0;
        /// @brief Adds several types to the context.
        virtual void    addTypes(const QStringList &types) = 0;
        /// @see addTypes
        inline  void    addType(const QString &type) { addTypes(QStringList(type)); }
        /// @brief Removes the types in the list from the context.
        virtual void    removeTypes(const QStringList &types) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IContext, "cc.lightbird.IContext")

#endif // LIGHTBIRD_ICONTEXT_H
