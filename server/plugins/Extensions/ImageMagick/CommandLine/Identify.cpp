#include <QProcess>
#include <QFileInfo>
#include <QTemporaryFile>

#include "Identify.h"
#include "LightBird.h"

Identify::Identify(LightBird::IApi *a)
    : api(a)
{
    IIdentify::_types << LightBird::IIdentify::IMAGE;
    // Get the path to ImageMagick from the configuration of the plugin
    this->imageMagickPath = this->api->configuration(true).get("image_magick_path");
    // The default path
    if (this->imageMagickPath.isEmpty())
        this->imageMagickPath = this->api->getPluginPath() + "/ImageMagick";
    this->binaryName = "identify";
    // Ensure that the identify binary exists
    QString path = this->imageMagickPath + "/" + this->binaryName;
    if (QFileInfo(path).isFile() || QFileInfo(path + ".exe").isFile())
        this->isInitialized();
    else
        LOG_ERROR("Could not found the identify binary of ImageMagick", Properties("path", path).toMap(), "Identify", "Identify");
    if (!(this->timeout = this->api->configuration(true).get("identify_timeout").toUInt()))
        this->timeout = 5000;
}

Identify::~Identify()
{
}

bool    Identify::identify(const QString &file, LightBird::IIdentify::Information &information)
{
    QProcess                process;
    QStringList             arguments;
    QString                 output;
    int                     i;

    // Check that the file exists
    // If the size of the file is greater than 50M, it shouldn't be an image
    if (!QFileInfo(file).isFile() || QFileInfo(file).size() > 50000000)
        return (false);
    information.data.clear();
    information.type = LightBird::IIdentify::OTHER;
    // Defines the command line
    arguments << "-verbose" << file;
    // Launch ImageMagick
    process.start(this->imageMagickPath + "/" + this->binaryName, arguments);
    process.waitForStarted();
    // Wait until the identification has finished
    if (!process.waitForFinished(this->timeout))
    {
        // If it is too long, the process is killed. This happends when the file is not an image (a video or a music).
        process.kill();
        process.waitForFinished(1000);
    }
    output = process.readAllStandardOutput();
    // If the exit code is not 0, an error occurred
    if (process.exitCode())
    {
        QString output;
        if (!output.isEmpty())
            output += "\n";
        output += process.readAllStandardError();
        LOG_DEBUG("An error occurred while executing ImageMagick", Properties("programOutput", output).add("exitCode", QString::number(process.exitCode())).add("commandLine", this->imageMagickPath + "/" + this->binaryName + " " + arguments.join(" ")).toMap(), "Identify", "identify");
        return (false);
    }
    LOG_TRACE("ImageMagick command line", Properties("commandLine", this->imageMagickPath + "/" + this->binaryName + " " + arguments.join(" ")).toMap(), "Identify", "identify");
    // Parse the data of the file
    output = output.trimmed().remove('\r');
    QStringListIterator it(output.split("\n"));
    while (it.hasNext())
    {
        if ((i = it.peekNext().indexOf(':')) > 0)
            this->_addData(information, it.peekNext().left(i).trimmed().toLower(), it.peekNext().right(it.peekNext().size() - i - 1).trimmed());
        it.next();
    }
    if (information.data.isEmpty())
        return (false);
    information.type = LightBird::IIdentify::IMAGE;
    return (true);
}

void    Identify::_addData(LightBird::IIdentify::Information &information, const QString &key, const QString &value)
{
    int i;

    if (key.isEmpty() || value.isEmpty() || value == "Undefined")
        return ;
    else if (key == "format" && value.contains('('))
    {
        i = value.indexOf('(');
        information.data.insert(key, value.left(value.indexOf('(')).trimmed());
        information.data.insert("format name", value.mid(i + 1, value.size() - i - 2));
    }
    else if (key == "format")
        information.data.insert(key, value);
    else if (key == "geometry" && value.contains('x'))
    {
        i = value.indexOf('x');
        information.data.insert("width", value.left(i));
        if (value.contains('+'))
            information.data.insert("height", value.mid(i + 1, value.size() - i - 1 - (value.size() - value.indexOf('+'))));
        else
            information.data.insert("height", value.right(value.size() - value.indexOf('x') - 1));
    }
    else if (key == "resolution")
        information.data.insert(key, value);
    else if (key == "print size")
        information.data.insert(key, value);
    else if (key == "compression")
        information.data.insert(key, value);
    else if (key == "quality")
        information.data.insert(key, value);
    else if (key == "depth")
        information.data.insert(key, value);
    else if (key == "red")
        information.data.insert(key, value);
    else if (key == "green")
        information.data.insert(key, value);
    else if (key == "blue")
        information.data.insert(key, value);
    else if (key == "colorspace")
        information.data.insert(key, value);
    else if (key == "background color")
        information.data.insert(key, value);
    else if (key == "transparent color")
        information.data.insert(key, value);
    else if (key == "number pixels")
        information.data.insert("pixels", value);
}
