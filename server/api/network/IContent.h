#ifndef ICONTENT_H
# define ICONTENT_H

# include <QByteArray>
# include <QFile>
# include <QTemporaryFile>
# include <QVariant>

namespace LightBird
{
    /// @brief This object allows to store data through different ways, and read/write on them.
    class IContent
    {
    public:
        virtual ~IContent() {}

        /// @brief List the available storages types.
        enum Storage
        {
            BYTEARRAY,    ///< The content is stored in the memory as a QByteArray. Recommended for short contents.
            VARIANT,      ///< The content is stored in the memory as a QVariant. Useful for representing structured data. Be careful to not store too large data.
            FILE,         ///< The content is stored in a file that must not be removed. Recommended to store a large amount of persistant data.
            TEMPORARYFILE ///< The content is stored in a temporary file that is removed with this object. Recommended to store a large amount of temporary data.
        };

        /// @brief Returns how the content is stored.
        virtual LightBird::IContent::Storage getStorage() const = 0;
        /// @brief Sets how the content is stored. Calling this method will clear all
        /// the stored contents (using clear()). Only one type of storage can be used
        /// at the same time.
        /// @param fileName : If the storage is FILE, this paramater defines the file name.
        /// If the storage is TEMPORARYFILE, this paramater defines the temporary file template.
        /// See QTemporaryFile for details.
        virtual LightBird::IContent &setStorage(LightBird::IContent::Storage storage, const QString &fileName = "") = 0;
        /// @brief Returns all or a part of the content, depending on the given size.
        /// The content is returned as a QByteArray of the given size, or less if all
        /// the data has been returned. The begin of the returned content depends on
        /// the position of the read pointer. To get the content of a VARIANT, one must
        /// call getVariant() instead of this method.
        /// @param size : The size of the content to return. Zero will return all the content.
        virtual QByteArray      getContent(quint64 size = 0) = 0;
        /// @brief Add the given content.
        /// @param content : The data to add to the content.
        /// @param append : If true, the new content is added at the end of the existing content.
        /// Otherwise the content is replaced. To modified a VARIANT, use getVariant() instead
        /// of this method.
        virtual void            setContent(const QByteArray &content, bool append = true) = 0;
        /// @brief Allows one to modify the byteArray. NULL is returned if the storage is not BYTEARRAY.
        virtual QByteArray      *getByteArray() = 0;
        /// @brief Allows one to modify the variant. NULL is returned if the storage is not VARIANT.
        virtual QVariant        *getVariant() = 0;
        /// @brief Allows one to modify the file. NULL is returned if the storage is not FILE.
        virtual QFile           *getFile() = 0;
        /// @brief Allows one to modify the temporary file. NULL is returned if the storage is not TEMPORARYFILE.
        virtual QTemporaryFile  *getTemporaryFile() = 0;
        /// @brief Returns the size of the content. Returns 0 with VARIANT.
        virtual qint64          size() const = 0;
        /// @brief Returns the current position of the read pointer.
        virtual qint64          getSeek() const = 0;
        /// @brief Sets the position of the read pointer.
        virtual void            setSeek(qint64 position) = 0;
        /// @brief Removes all the contents, and put the instance in its initial state.
        virtual void            clear() = 0;
    };
}

#endif // ICONTENT_H
