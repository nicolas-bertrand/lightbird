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
    /// @brief Write the data on the stream.
    virtual bool    write(QByteArray &data, Client *client) = 0;
};

#endif // IREADWRITE_H
