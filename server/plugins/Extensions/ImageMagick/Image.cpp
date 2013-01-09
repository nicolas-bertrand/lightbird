#include <QProcess>
#include <QFileInfo>
#include <QTemporaryFile>

#include "Image.h"
#include "LightBird.h"
#include "Properties.h"

Image::Image(LightBird::IApi *a)
    : api(a)
{
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
    if (!(this->timeout = this->api->configuration(true).get("convert_timeout").toUInt()))
        this->timeout = 5000;
}

Image::~Image()
{
}

bool    Image::convert(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width, unsigned int height, float quality)
{
    QString         extension = LightBird::getImageExtension(format);
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
        tmp.setFileTemplate(this->fileTemplate + extension);
        if (!tmp.open())
        {
            LOG_DEBUG("Error with QTemporaryFile::open", "Image", "convert");
            return (false);
        }
        destination = tmp.fileName();
    }
    // Defines the command line
    commandLine = this->imageMagickPath + "/" + this->binaryName + " " + source + " " + this->_resize(width, height) + this->_quality(quality) + destination;
    properties.add("commandLine", commandLine);
    if (!destination.contains(QRegExp("\\." + extension + "$")))
        commandLine += "." + extension;
    // Start the converter
    process.start(commandLine);
    process.waitForStarted();
    // Wait until the conversion has finished
    if (!process.waitForFinished(this->timeout))
    {
        // If it is too long, the process is killed. This happends when the file is not an image (a video or a music).
        process.kill();
        process.waitForFinished(1000);
    }
    // If the exit code is not 0, an error occured
    if (process.exitCode())
    {
        QString output;
        if (!(output = process.readAllStandardOutput()).isEmpty())
            output += "\n";
        output += process.readAllStandardError();
        LOG_DEBUG("An error occured while executing ImageMagick", properties.add("programOutput", output).add("exitCode", QString::number(process.exitCode())).toMap(), "Image", "convert");
        if (replaceSource)
            destination = "";
        return (false);
    }
    LOG_TRACE("ImageMagick command line", properties.toMap(), "Image", "convert");
    // Add the extension at the end of the destination file name
    if (!destination.endsWith("." + extension))
        destination += "." + extension;
    // Replace the source by the destination if the destination was empty
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

QString Image::_quality(float quality)
{
    quality = qRound(quality * 100);
    if (quality < 0)
        return ("");
    if (quality < 1)
        quality = 1;
    if (quality > 100)
        quality = 100;
    return ("-quality " + QString::number(quality) + " ");
}
