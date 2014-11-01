#ifndef LIGHTBIRD_IPREVIEW_H
# define LIGHTBIRD_IPREVIEW_H

# include <QString>

# include "IIdentify.h"
# include "IImage.h"

namespace LightBird
{
    /// @brief Allows plugins to get a preview image of a file.
    class IPreview
    {
    public:
        virtual ~IPreview() {}

        /// @brief Returns the list of the types the extension can generate previews for.
        inline const QList<LightBird::IIdentify::Type> &types() { return _types; }

        /// @brief Generates a preview image of a file if possible.
        /// If the width and the height are 0, the original size is keeped.
        /// @param source : The name of the file for which the preview image will be generated.
        /// @param destination : The path to the file that will contains the preview without its extension.
        /// The extension will be added, depending on the format.
        /// @param format : The format of the preview image that will be generate.
        /// If the format is NONE, the default preview format is used.
        /// @param width : The width of the preview. If it is 0, it will be proportional to the height.
        /// @param height : The height of the preview. If it is 0, it will be proportional to the width.
        /// @param position : For a video this parameter could be the time where the preview is captured.
        /// @param quality : The quality factor must be in the range 0 to 1 or -1.
        /// Specify 0 to obtain small compressed files, 1 for large uncompressed files,
        /// and -1 (the default) to use the default settings.
        /// @return False if the extension doesn't know how to make a preview from the file.
        /// True is returned if the preview has been generated.
        virtual bool    generate(const QString &source, QString &destination, LightBird::IImage::Format format = LightBird::IImage::NONE, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0, float quality = -1) = 0;

    protected:
        ///< The list of the types the extension can generate previews for.
        QList<LightBird::IIdentify::Type> _types;
    };
}

Q_DECLARE_INTERFACE(LightBird::IPreview, "cc.lightbird.IPreview")

#endif // LIGHTBIRD_IPREVIEW_H
