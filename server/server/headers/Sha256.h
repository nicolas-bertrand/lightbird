#ifndef SHA256_H
# define SHA256_H

#include <QByteArray>

/// @brief Implementation of SHA-256.
/// @author jagatsastry.nitk@gmail.com
namespace Sha256
{
    /// @brief Returns the hash of data in hex.
    QByteArray  hash(QByteArray data);
};

#endif // SHA256_H
