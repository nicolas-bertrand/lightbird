#ifndef IPREVIEW_H
# define IPREVIEW_H

# include <QString>

# include "IImage.h"

namespace LightBird
{
    /// @brief Allows plugins to get a preview image of a file.
    class IPreview
    {
    public:
        virtual ~IPreview() {}

        /// @brief Generates a preview image of a file if possible. If the width
        /// and the height are 0, the original size is keeped.
        /// @param source : The name of the file for which the preview image will be generated.
        /// @param destination : The path to the file that will contains the preview without its extension.
        /// The extension will be added, depending on the format.
        /// @param format : The format of the preview image that will be generate.
        /// @param width : The width of the preview. If it is 0, it will be proportional to the height.
        /// @param height : The height of the preview. If it is 0, it will be proportional to the width.
        /// @param position : For a video, this parameter could be the time where the preview is captured.
        /// @return False if the extension doesn't know how to make a preview from the file.
        /// True is returned if the preview has been generated.
        virtual bool    generate(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0) = 0;
    };
}

#endif // IPREVIEW_H
