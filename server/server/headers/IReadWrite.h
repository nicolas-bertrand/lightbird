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
    /// @brief Writes the data to the stream.
    /// Client::bytesWritten have to be called afterward.
    virtual void    write(Client *client, const QByteArray &data) = 0;
};

#endif // IREADWRITE_H
