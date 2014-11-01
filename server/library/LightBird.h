#ifndef LIGHTBIRD_H
# define LIGHTBIRD_H

# include <iostream>
# include <QByteArray>
# include <QImage>
# include <QObject>
# include <QString>

# include "IIdentify.h"
# include "IImage.h"

# include "Export.h"
# include "Initialize.h"
# include "Configuration.h"
# include "Properties.h"
# include "Mutex.h"
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
    /// @brief Converts an address into a hexadecimal lowercase string.
    LIB QString     addressToString(void *address);

    /// @brief Provides a fast way to access the general configuration of the server.
    LIB inline const Configuration &c() { return Configuration::get(); }

    /// @brief LightBird's implementation of Qt::copy.
    LIB bool        copy(const QString &source, const QString &destination);

    /// @brief Does the same job as QDir::cleanPath but removes also the "\" under Linux.
    /// @param removeFirstSlash : Removes the first char if it is "/".
    LIB QString     cleanPath(const QString &path, bool removeFirstSlash = false);

    /// @brief Converts the file type to a string (IIdentify::AUDIO becomes "audio"...).
    LIB QString     fileTypeToString(IIdentify::Type type);

    /// @brief Converts the file type from a string ("audio" becomes IIdentify::AUDIO...).
    LIB IIdentify::Type fileTypeFromString(const QString &type);

    /// @brief Identifies a file in the database. The identification is performed
    /// in a dedicated thread, so this method returns immediatly.
    /// The result will be inserted in the database.
    ///
    /// Calls the IIdentify interface of each plugins that implement it. Then
    /// tries to mix all the data gathered in order to get the final list of
    /// information on the file and its type.
    /// @param fileId : The id of the fine to identify in the database.
    LIB void        identify(const QString &fileId);

    /// @brief Allows to get information on a file. The identification is
    /// performed in the current thread, and the information parameter is
    /// filled with the result.
    ///
    /// Calls the IIdentify interface of each plugins that implement it. Then
    /// tries to mix all the data gathered in order to get the final list of
    /// information on the file and its type.
    /// @param filePath : The path to the file to idenfity.
    /// @param information : This parameter is filled with the data collected.
    LIB void        identify(const QString &filePath, LightBird::IIdentify::Information &information);

    /// @brief Returns true if the file or directory name is valid and can be safely used.
    LIB bool        isValidName(const QString &objectName);

    /// @brief Returns a new Universally Unique Identifier.
    LIB QString     createUuid();

    /// @brief Returns the MIME of the file based on its extension.
    /// The default MIME type is "application/octet-stream".
    /// @param fileName : The name of the file or just its extension.
    LIB QString     getFileMime(const QString &fileName);

    /// @brief Returns the type of the file based on its extension.
    /// @param fileName : The name of the file or just its extension.
    LIB IIdentify::Type getFileType(const QString &fileName);

    /// @brief Returns the filesPath, which is the relative path to the
    /// directory that stores the files managed by the server. The filesPath
    /// is defined by the configuration node "filesPath".
    /// @param finalSlash : If true, a "/" is added at the end of the filesPath.
    LIB QString     getFilesPath(bool finalSlash = true);

    /// @brief Returns the extension that corresponds to the format.
    /// @param dot : If the extension have to start with a dot.
    LIB QString     getImageExtension(LightBird::IImage::Format format, bool dot = false);

    /// @brief Returns the image format based on the string.
    /// @param format : Can be an extension, a file name or a format name.
    /// @param defaultFormat : The format returned if the format in parameter is unknown.
    LIB LightBird::IImage::Format getImageFormat(QString format, LightBird::IImage::Format defaultFormat = LightBird::IImage::NONE);

    /// @brief Returns the list of the ports in the string.
    /// The ports can be separated by any character excluding '-',
    /// which is used to represent a range of ports between two numbers.
    /// @param max : The maximum number of ports returned.
    LIB QList<ushort> parsePorts(QString ports, int max = 100);

    /// @brief Generates a preview image of the file if possible.
    /// Calls all the plugins that implements IPreview in order to generate
    /// the preview of a file. A cache system is also implemented in order to
    /// not generate a new preview at each calls.
    /// @param fileId : The id of the file for which the preview image will be generated.
    /// @param format : The format of the preview image that will be generate.
    /// If the format is NONE, the default preview format is used.
    /// @param width : The width of the preview. If it is 0, it will be proportional to the height.
    /// @param height : The height of the preview. If it is 0, it will be proportional to the width.
    /// @param position : For a video, this parameter could be the time where the preview is captured.
    /// @param quality : The quality factor must be in the range 0 to 1 or -1.
    /// Specify 0 to obtain small compressed files, 1 for large uncompressed files,
    /// and -1 (the default) to use the default settings.
    /// @return The path to the generated preview. This file should not been deleted (for cache purpose).
    /// If empty, no preview could have been generated for the source file.
    LIB QString     preview(const QString &fileId, LightBird::IImage::Format format = LightBird::IImage::NONE, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0, float quality = -1);

    /// @brief Saves the image in the requested format if Qt supports it.
    /// If the width and the height are 0, the original size is keeped.
    /// @param fileName : The name of the file in which the image is saved.
    /// The extension of the format is added if not already present.
    /// @param width : If the width is 0, it will be proportional to the height.
    /// @param height : If the height is 0, it will be proportional to the width.
    /// @param quality : The quality factor must be in the range 0 to 1 or -1.
    /// Specify 0 to obtain small compressed files, 1 for large uncompressed files,
    /// and -1 (the default) to use the default settings.
    /// @return True on success.
    LIB bool        saveImage(QImage &image, QString &fileName, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, float quality = -1);

    /// @brief Generates the SHA-256 of data.
    LIB QByteArray  sha256(const QByteArray &data);

    /// @brief Replaces the unprintable ascii characters of data by a dot (by default)
    /// and truncates the data if necessary.
    /// @param data : The data to simplify.
    /// @param replace : Replaces the unprintable ascii characters by this value.
    /// @param maxSize : If the data size exceeds maxSize it is truncated. If the
    /// value is zero the data are not truncated.
    /// @return The simplified data.
    LIB QByteArray  simplify(QByteArray data, char replace = '.', quint64 maxSize = 2000);

    /// @brief A cross-platform sleep.
    /// @param The number of milliseconds to sleep.
    LIB void        sleep(unsigned long time);

    /// @brief Seeds the rand() of the current thread.
    LIB void        srand();

    /// @brief Converts a string to a quint64 using the character at the end of the string.
    /// This character could be K (Kilobyte), M (Megabyte), G (Gigabyte), or T (Terabyte).
    /// For example "42K" is converted to 43008 (42 * 1024), and "42" is converted to 42.
    /// @param string : The string to convert.
    /// @return The result of the conversion.
    LIB quint64     stringToBytes(const QString &str);

    /// @brief Returns the current number of milliseconds since epoch in UTC. This
    /// method is needed because QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()
    /// returns the same value as QDateTime::currentDateTime().toMSecsSinceEpoch().
    LIB qint64      currentMSecsSinceEpochUtc();
}

#endif // LIGHTBIRD_H
