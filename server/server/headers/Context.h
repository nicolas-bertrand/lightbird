#ifndef CONTEXT_H
# define CONTEXT_H

# include <QObject>
# include <QList>
# include <QMap>
# include <QString>
# include <QStringList>

/// @brief Stores a context of a plugin. A context is a set of conditions that
/// defines if the interfaces of a plugin has to be called in a particular situation.
class Context : public QObject
{
    Q_OBJECT

public:
    Context(QObject *parent = 0);
    ~Context();
    Context(const Context &context);
    Context &operator=(const Context &context);
    bool    operator==(const Context &context);

    /// @brief Add a transport to the context.
    void    setTransport(const QString &transport);
    /// @brief Add a protocol to the context.
    void    setProtocol(const QString &protocol);
    /// @brief Add a port to the context.
    void    setPort(unsigned short port);
    /// @brief Add a method to the context.
    void    setMethod(const QString &method);
    /// @brief Add a type to the context.
    void    setType(const QString &type);

    /// @brief Check that the context given in parameter is valid under this context.
    bool    isValid(const QString &transport, const QStringList &protocols, unsigned short port) const;
    /// @see isValid
    bool    isValid(const QString &transport, const QStringList &protocols, unsigned short port, const QString &method, const QString &type) const;
    /// @brief Converts the context into a QMap
    QMap<QString, QString>  toMap() const;

private:
    QString                 transport;  ///< The transport protocol to use ("TCP" or "UDP").
    QStringList             protocols;  ///< The list of communication protocols known by the plugin ("HTTP", "UPnP").
    QList<unsigned short>   ports;      ///< The list of ports listenned by the plugin.
    QStringList             methods;    ///< The list of request method available for the plugin.
    QStringList             types;      ///< The list of file types handled by the plugin (MIME types).
};

#endif // CONTEXT_H
