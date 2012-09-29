#ifndef IREADWRITE_H
# define IREADWRITE_H

# include <QByteArray>
# include <QObject>

class Client;

/// @brief This interface allows to abstract the reveive and send operations
/// in TCP and UDP between the Client and the network back-ends.
class IReadWrite
{
public:
    virtual ~IReadWrite() {}

    /// @brief Tells the network that the client is ready to read more data.
    /// The data have to be filled in the reference returned by Client::getData,
    /// from any thread. Client::bytesRead have to be called afterward.
    virtual void    read(Client *client) = 0;
    /// @brief Write the data on the stream. This method takes ownership
    /// of the data, ie it takes the responsability to delete them.
    /// Client::bytesWriting and bytesWritten have to be called afterward.
    virtual bool    write(QByteArray *data, Client *client) = 0;
    /// @brief This method can be reimplemented in order to connect the
    /// server to a client.
    virtual bool    connect(Client *)
    {
        return (true);
    }
};

#endif // IREADWRITE_H
