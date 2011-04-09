#ifndef IPREVIEWS_H
# define IPREVIEWS_H

# include <QString>

# include "IImage.h"

/// @brief Calls all the plugins that implements IPreview in order to generate
/// the preview of a file. A cache system should also be implemented in order
/// to not generate a new preview at each calls.
namespace LightBird
{
    class IPreviews
    {
    public:
        virtual ~IPreviews() {}

        /// @brief Generates a preview image in a file if possible. If the width
        /// and the height are 0, the original size is keeped.
        /// @param source : The id of the file for which the preview image will be generated.
        /// @param format : The format of the preview image that will be generate.
        /// @param width : The width of the preview. If it is 0, it will be proportional to the height.
        /// @param height : The height of the preview. If it is 0, it will be proportional to the width.
        /// @param position : For a video, this parameter could be the time where the preview is captured.
        /// @return The path to the generated preview. This file should not been deleted (for cache purpose).
        /// If empty, no preview could have been generated for the source file.
        virtual QString previews(const QString &fileId, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0) = 0;
    };
}

#endif // IPREVIEWS_H
