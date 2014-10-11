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
    WriteBuffer(Client *_client, const QByteArray &data, qint64 written);
    ~WriteBuffer();

    /// @brief Returns the next data to send.
    inline const char  *getData() const { return this->_data.data() + this->_written; }

    /// @brief Returns the number of bytes remaining.
    inline qint64 getSize() const { return qMax(this->_data.size() - this->_written, qint64(0)); }

    /// @brief Return the total size of the data managed by the WriteBuffer.
    inline qint64 getTotalSize() const { return _data.size(); }

    /// @brief Increases the number of bytes written, and sets writing to true.
    inline void bytesWritten(qint64 written) { this->_written += written; }

    /// @brief Returns the number of bytes written so far.
    inline qint64 bytesWritten() const { return _written; }

    /// @brief Returns true if all the data have been written.
    inline bool isWritten() const { return this->_written >= this->_data.size(); }

private:
    WriteBuffer(const WriteBuffer &);
    WriteBuffer &operator=(const WriteBuffer &);

    Client *_client; ///< The client to which the data will be written.
    const QByteArray &_data; ///< The data to write.
    qint64 _written; ///< The number of bytes already written.
};

#endif // WRITEBUFFER_H
