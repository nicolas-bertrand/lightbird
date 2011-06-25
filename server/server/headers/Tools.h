#ifndef TOOLS_H
# define TOOLS_H

# include <QString>

namespace Tools
{
    /// @brief LightBird's implementation of Qt::copy.
    bool        copy(const QString &source, const QString &destination);
    /// @brief Does the same job as QDir::cleanPath but removes also the "\" under Linux.
    QString     cleanPath(const QString &path);
    /// @brief Returns a new Universally Unique Identifier.
    QString     createUuid();
    /// @brief Replaces the unprintable ascii characteres of data by a dot (by default)
    /// and truncates the data if necessary.
    /// @param data : The data to simplify.
    /// @param replace : Replaces the unprintable ascii characteres by this value.
    /// @param maxSize : If the data size exceeds maxSize it is truncated. If the
    /// value is zero the data are not truncated.
    /// @return The simplified data.
    QByteArray  simplify(QByteArray data, char replace = '.', quint64 maxSize = 2000);
};

#endif // TOOLS_H
