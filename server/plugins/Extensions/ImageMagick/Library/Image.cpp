#include <QFileInfo>
#include <QTemporaryFile>
#include <Magick++.h>

#include "Image.h"
#include "LightBird.h"
#include "Properties.h"

Image::Image(LightBird::IApi *a)
    : api(a)
{
    this->fileTemplate = this->api->configuration().get("temporaryPath") + "/" + "XXXXXX";
    this->isInitialized();
}

Image::~Image()
{
}

bool    Image::convert(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width, unsigned int height, float quality)
{
    QString         extension = LightBird::getImageExtension(format);
    Properties      properties;
    QTemporaryFile  tmp;
    bool            replaceSource = false;

    // Checks that the source exists
    if (!QFileInfo(source).isFile())
        return (false);
    if (source == destination)
        destination.clear();
    // If destination is empty, the result is stored in a temporary file
    if (destination.isEmpty())
    {
        replaceSource = true;
        tmp.setFileTemplate(this->fileTemplate + extension);
        if (!tmp.open())
        {
            LOG_DEBUG("Error with QTemporaryFile::open", "Image", "convert");
            return (false);
        }
        destination = tmp.fileName();
    }
    // Adds the extension at the end of the destination file name
    if (!destination.endsWith("." + extension))
        destination += "." + extension;

    // Converts the image
    try
    {
        Magick::Image image(source.toStdString());
        image.resize(Magick::Geometry(width, height));
        image.quality(_quality(quality));
        image.write(destination.toStdString());
    }
    catch(Magick::Exception &e)
    {
        LOG_DEBUG("An error occured while converting the image", properties.add("what", e.what()).add("source", source).add("destination", destination).toMap(), "Image", "convert");
        if (replaceSource)
            destination = "";
        return (false);
    }

    // Replaces the source by the destination if the destination was empty
    if (replaceSource)
    {
        if (!QFile::remove(source))
        {
            LOG_DEBUG("Unable to remove the source file", properties.add("source", source).toMap(), "Image", "convert");
            return (false);
        }
        if (!QFile::copy(destination, source))
        {
            LOG_DEBUG("Unable to replace the source by the destination", properties.add("source", source).add("destination", destination).toMap(), "Image", "convert");
            return (false);
        }
        destination = source;
    }
    return (true);
}

bool    Image::generate(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width, unsigned int height, unsigned int, float quality)
{
    if (destination.isEmpty() || source.endsWith(".txt"))
        return (false);
    return (this->convert(source, destination, format, width, height, quality));
}

unsigned int Image::_quality(float quality)
{
    quality = qRound(quality * 100);
    if (quality < 0)
        return (75);
    else if (quality < 1)
        quality = 1;
    else if (quality > 100)
        quality = 100;
    return (quality);
}
