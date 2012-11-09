#include <QProcess>
#include <QFileInfo>
#include <QTemporaryFile>

#include "Image.h"
#include "Properties.h"

Image::Image(LightBird::IApi *a) : api(a)
{
    this->extensions[LightBird::IImage::BMP] = "bmp";
    this->extensions[LightBird::IImage::GIF] = "gif";
    this->extensions[LightBird::IImage::JPEG] = "jpg";
    this->extensions[LightBird::IImage::PNG] = "png";
    this->extensions[LightBird::IImage::TGA] = "tga";
    this->extensions[LightBird::IImage::TIFF] = "tiff";
    // Get the path to ImageMagick from the configuration of the plugin
    this->imageMagickPath = this->api->configuration(true).get("image_magick_path");
    // The default path
    if (this->imageMagickPath.isEmpty())
        this->imageMagickPath = this->api->getPluginPath() + "/ImageMagick";
    this->binaryName = "convert";
    this->fileTemplate = this->api->configuration().get("temporaryPath") + "/" + "XXXXXX";
    // Ensure that the identify binary exists
    QString path = this->imageMagickPath + "/" + this->binaryName;
    if (QFileInfo(path).isFile() || QFileInfo(path + ".exe").isFile())
        this->isInitialized();
    else
        LOG_ERROR("Could not found the convert binary of ImageMagick", Properties("path", path).toMap(), "Image", "Image");
}

Image::~Image()
{
}

bool    Image::convert(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width, unsigned int height)
{
    QProcess        process;
    QString         commandLine;
    Properties      properties;
    QTemporaryFile  tmp;
    bool            replaceSource = false;

    // Check that the source exists
    if (!QFileInfo(source).isFile() || source == destination)
        return (false);
    // If destination is empty, the result is stored in a temporary file
    if (destination.isEmpty())
    {
        replaceSource = true;
        tmp.setFileTemplate(this->fileTemplate + this->extensions[format]);
        if (!tmp.open())
        {
            LOG_ERROR("Error with QTemporaryFile::open", "Image", "convert");
            return (false);
        }
        destination = tmp.fileName();
    }
    // Defines the command line
    commandLine = this->imageMagickPath + "/" + this->binaryName + " " + source + " " + this->_resize(width, height) + destination;
    properties.add("commandLine", commandLine);
    if (!destination.contains(QRegExp("\\." + this->extensions[format] + "$")))
        commandLine += "." + this->extensions[format];
    // Start the converter
    process.start(commandLine);
    process.waitForStarted();
    // Wait until the conversion has finished
    process.waitForFinished();
    // If the exit code is not 0, an error occured
    if (process.exitCode())
    {
        QString output;
        if (!(output = process.readAllStandardOutput()).isEmpty())
            output += "\n";
        output += process.readAllStandardError();
        this->api->log().debug("An error occured while executing ImageMagick", properties.add("programOutput", output).add("exitCode", QString::number(process.exitCode())).toMap(), "Image", "convert");
        if (replaceSource)
            destination = "";
        return (false);
    }
    LOG_TRACE("ImageMagick command line", properties.toMap(), "Image", "convert");
    // Add the extension at the end of the destination file name
    if (!destination.endsWith("." + this->extensions[format]))
        destination += "." + this->extensions[format];
    // Replace the source by the destination if the destination was empty
    if (replaceSource)
    {
        if (!QFile::remove(source))
        {
            LOG_ERROR("Unable to remove the source file", properties.add("source", source).toMap(), "Image", "convert");
            return (false);
        }
        if (!QFile::copy(destination, source))
        {
            LOG_ERROR("Unable to replace the source by the destination", properties.add("source", source).add("destination", destination).toMap(), "Image", "convert");
            return (false);
        }
        destination = source;
    }
    return (true);
}

bool    Image::generate(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width, unsigned int height, unsigned int)
{
    if (destination.isEmpty() || source.endsWith(".txt"))
        return (false);
    return (this->convert(source, destination, format, width, height));
}

QString Image::_resize(unsigned int width, unsigned int height)
{
    if (!width && !height)
        return ("");
    if (!width)
        return ("-resize x" + QString::number(height) + " ");
    if (!height)
        return ("-resize " + QString::number(width) + " ");
    return ("-resize " + QString::number(width) + "x" + QString::number(height) + "! ");
}