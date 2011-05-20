#ifndef TOOLS_H
# define TOOLS_H

# include <QString>

namespace Tools
{
    /// @brief LightBird's implementation of Qt::copy.
    bool    copy(const QString &source, const QString &destination);
    /// @brief Does the same as QDir::cleanPath but removes also the "\" under Linux.
    QString cleanPath(const QString &path);
};

#endif // TOOLS_H
