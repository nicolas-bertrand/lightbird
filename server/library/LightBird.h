#ifndef LIGHTBIRD_H
# define LIGHTBIRD_H

# include <QByteArray>
# include <QObject>
# include <QString>

# include "IImage.h"

# include "Export.h"
# include "Initialize.h"
# include "Properties.h"
# include "SmartMutex.h"
# include "TableAccounts.h"
# include "TableCollections.h"
# include "TableDirectories.h"
# include "TableEvents.h"
# include "TableFiles.h"
# include "TableGroups.h"
# include "TableLimits.h"
# include "TablePermissions.h"
# include "TableTags.h"

namespace LightBird
{
    /// @brief LightBird's implementation of Qt::copy.
    LIB bool        copy(const QString &source, const QString &destination);
    /// @brief Does the same job as QDir::cleanPath but removes also the "\" under Linux.
    /// @param removeFirstSlash : Removes the first char if it is "/".
    LIB QString     cleanPath(const QString &path, bool removeFirstSlash = false);
    /// @brief Returns true if the file or directory name is valid and can be safely used.
    LIB bool        isValidName(const QString &objectName);
    /// @brief Returns a new Universally Unique Identifier.
    LIB QString     createUuid();
    /// @brief Returns the filesPath, which is the relative path to the
    /// directory that stores the files managed by the server. The filesPath
    /// is defined by the configuration node "filesPath".
    /// @param finalSlash : If true, a "/" is added at the end of the filesPath.
    LIB QString     getFilesPath(bool finalSlash = true);
    /// @brief Calls all the plugins that implements IPreview in order to generate
    /// the preview of a file. A cache system is also implemented in order to
    /// not generate a new preview at each calls.
    ///
    /// Generates a preview image of a file if possible. If the width and the height are 0,
    /// the original size is keeped.
    /// @param fileId : The id of the file for which the preview image will be generated.
    /// @param format : The format of the preview image that will be generate.
    /// @param width : The width of the preview. If it is 0, it will be proportional to the height.
    /// @param height : The height of the preview. If it is 0, it will be proportional to the width.
    /// @param position : For a video, this parameter could be the time where the preview is captured.
    /// @return The path to the generated preview. This file should not been deleted (for cache purpose).
    /// If empty, no preview could have been generated for the source file.
    LIB QString     preview(const QString &fileId, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0);
    /// @brief Implementation of SHA-256.
    /// @author jagatsastry.nitk@gmail.com
    /// @return The hash of data in hex.
    LIB QByteArray  sha256(QByteArray data);
    /// @brief Replaces the unprintable ascii characteres of data by a dot (by default)
    /// and truncates the data if necessary.
    /// @param data : The data to simplify.
    /// @param replace : Replaces the unprintable ascii characteres by this value.
    /// @param maxSize : If the data size exceeds maxSize it is truncated. If the
    /// value is zero the data are not truncated.
    /// @return The simplified data.
    LIB QByteArray  simplify(QByteArray data, char replace = '.', quint64 maxSize = 2000);
    /// @brief A cross-platform sleep.
    /// @param The number of milliseconds to sleep.
    LIB void        sleep(unsigned long time);
}

#endif // LIGHTBIRD_H
