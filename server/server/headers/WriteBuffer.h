#ifndef WRITEBUFFER_H
# define WRITEBUFFER_H

# include <QByteArray>
# include <QObject>

# include "Client.h"

/// @brief Allows the write the data to a client in multiple time.
class WriteBuffer : public QObject
{
    Q_OBJECT

public:
    /// @param data : This object takes ownership of the data and will delete
    /// them in its destructor.
    WriteBuffer(Client *client, QByteArray *data, QThread *thread);
    ~WriteBuffer();

    /// @brief Returns the next data to send.
    const char  *getData() const;
    /// @brief Returns the number of bytes remaining.
    int         getSize() const;
    /// @brief Return the total size of the data managed by the WriteBuffer.
    int         getTotalSize() const;
    /// @brief Increases the number of bytes written, and sets writing to true.
    void        bytesWriting(int written);
    /// @brief Returns true if data are being written.
    bool        isWriting() const;
    /// @brief Returns true if all the data have been written.
    bool        isWritten() const;

signals:
    /// @brief Emitted when a chunk of data have been written.
    void        bytesWritten();

private slots:
    /// @brief A chunk of data have been written on the socket.
    void        _bytesWritten();

private:
    WriteBuffer(const WriteBuffer &);
    WriteBuffer &operator=(const WriteBuffer &);

    Client      *client; ///< The client to which the data will be written.
    QByteArray  *data;   ///< The data to write.
    int         written; ///< The number of bytes already written.
    bool        writing; ///< True if data are being written.
};

#endif // WRITEBUFFER_H
