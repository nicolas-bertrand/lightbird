#ifndef TOOLS_H
# define TOOLS_H

# include <QString>

namespace Tools
{
    /// @brief LightBird's implementation of Qt::copy.
    bool    copy(const QString &source, const QString &destination);
    /// @brief Does the same job as QDir::cleanPath but removes also the "\" under Linux.
    QString cleanPath(const QString &path);
    /// @brief Returns a new Universally Unique Identifier.
    QString createUuid();
};

#endif // TOOLS_H
