#ifndef IREADWRITE_H
# define IREADWRITE_H

# include <QByteArray>
# include <QObject>

class Client;

/// @brief This interface allows a class to use an abstract read/write stream.
/// For example, this is used by the network to abstract the reveive and send
/// operations in TCP and UDP.
class IReadWrite
{
public:
    virtual ~IReadWrite() {}

    /// @brief Read from the stream. The data are stored in the parameter.
    virtual bool    read(QByteArray &data, Client *client) = 0;
    /// @brief Write the data on the stream. This method takes ownership
    /// of the data, ie it takes the responsability to delete them.
    virtual bool    write(QByteArray *data, Client *client) = 0;
    /// @brief This method can be reimplemented in order to connect the
    /// server to a client.
    virtual bool    connect(Client *)
    {
        return (true);
    }
};

#endif // IREADWRITE_H
