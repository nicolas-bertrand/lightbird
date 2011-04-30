#ifndef TOOLS_H
# define TOOLS_H

# include <QString>

namespace Tools
{
    /// @brief LightBird's implementation of Qt::copy.
    bool    copy(const QString &source, const QString &destination);
};

#endif // TOOLS_H
