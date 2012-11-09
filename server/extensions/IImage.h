#ifndef IIMAGE_H
# define IIMAGE_H

# include <QString>

namespace LightBird
{
    /// @brief Allows plugins to convert image files.
    class IImage
    {
    public:
        virtual ~IImage() {}

        /// @brief The available image formats.
        enum Format
        {
            BMP,
            GIF,
            JPEG,
            PNG,
            TGA,
            TIFF
        };

        /// @brief Converts the source image into the given format at the given size. If the width
        /// and the height are 0, the original size is keeped.
        /// @param source : The name of the image that will be converted.
        /// @param destination : The path to the file that will contains the converted image, without
        /// its extension. The extension will be added, depending on the format. If the destination
        /// is empty, the source is replaced (if the format is the same).
        /// @param format : The format in which the source will be converted.
        /// @param width : The width of the new image. If it is 0, it will be proportional to the height.
        /// @param height : The height of the new image. If it is 0, it will be proportional to the width.
        /// @return True is returned if the image has been successfully converted.
        virtual bool    convert(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0) = 0;
    };
}

#endif // IIMAGE_H
