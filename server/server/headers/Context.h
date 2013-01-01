#ifndef CONTEXT_H
# define CONTEXT_H

# include <QList>
# include <QMap>
# include <QObject>
# include <QString>
# include <QStringList>

# include "IClient.h"

/// @brief Stores a context of a plugin. A context is a set of conditions that
/// defines if the interfaces of a plugin has to be called in a particular situation.
class Context : public QObject
{
    Q_OBJECT

public:
    Context(const QString &idPlugin);
    ~Context();
    Context(const Context &context);
    Context &operator=(const Context &context);
    bool    operator==(const Context &context);

    /// @brief Sets the name of the context.
    void    setName(const QString &name);
    /// @brief Sets the mode of the context.
    void    setMode(QString mode);
    /// @brief Sets the transport protocol of the context.
    void    setTransport(QString transport);
    /// @brief Adds several protocols to the context.
    void    addProtocols(const QStringList &protocols);
    /// @brief Adds several ports to the context.
    void    addPorts(const QStringList &ports);
    /// @brief Adds several methods to the context.
    void    addMethods(const QStringList &methods);
    /// @brief Adds several types to the context.
    void    addTypes(const QStringList &types);
    /// @brief Ensures that the context name corresponds to a valid context
    /// instance in the plugin.
    /// @param contexts : The contexts of the plugin as returned by LightBird::IContexts.
    bool    checkName(QMap<QString, QObject *> &contexts);
    /// @brief Returns the plugin instance associated with the context.
    QObject *getInstance() const;

    /// @brief The informations against which the context is validated.
    struct Validator
    {
        Validator(QStringList &n, LightBird::IClient::Mode mo, LightBird::INetwork::Transport tr, QStringList &pr, ushort &po)
            : names(&n), mode(mo), transport(tr), protocols(&pr), port(po), method(NULL), type(NULL) {}
        const QStringList *names;
        const LightBird::IClient::Mode mode;
        const LightBird::INetwork::Transport transport;
        const QStringList *protocols;
        const ushort      &port;
        const QString     *method;
        const QString     *type;
    };

    /// @brief Validates the context against the informations of the validator.
    bool    isValid(const Context::Validator &validator) const;
    /// @brief Converts the context into a QMap.
    QMap<QString, QString>  toMap() const;

private:
    QString       idPlugin;        ///< The id of the plugin.
    QObject       *instance;       ///< The plugin instance associated with this context.
    QString       name;            ///< The name of the context.
    bool          allProtocols;    ///< The context matches all the protocols (HTTP, FTP...).
    QStringList   protocols;       ///< The list of communication protocols knowns by the plugin (not used when allProtocols is true).
    QList<ushort> ports;           ///< The list of ports listenned by the plugin.
    QStringList   methods;         ///< The list of methods available for the plugin.
    QStringList   types;           ///< The list of file types handled by the plugin (MIME types).
    bool          allModes;        ///< The context matches all the modes (CLIENT and SERVER).
    bool          allTransports;   ///< The context matches all the transport protocols (TCP and UDP).
    LightBird::IClient::Mode mode; ///< The connection mode of the client (not used when allModes is true).
    LightBird::INetwork::Transport transport; ///< The transport protocol to use (not used when allTransports is true).
};

#endif // CONTEXT_H
