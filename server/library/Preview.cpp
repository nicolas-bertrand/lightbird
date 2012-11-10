#include <QtPlugin>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
#include <QMutex>

#include "IPreview.h"

#include "Library.h"
#include "LightBird.h"
#include "Preview.h"

Preview::Preview()
{
    this->cacheEnabled = false;
    if (LightBird::Library::configuration().get("preview/cacheEnabled") == "true")
        this->cacheEnabled = true;
    if ((this->cachePath = LightBird::Library::configuration().get("preview/cachePath")).isEmpty())
        this->cachePath = "cache";
    this->cacheSizeLimit = LightBird::Library::configuration().get("preview/cacheSizeLimit").toUInt();
    this->extensions[LightBird::IImage::JPEG] = ".jpeg";
    this->extensions[LightBird::IImage::PNG] = ".png";
    this->extensions[LightBird::IImage::GIF] = ".gif";
    this->extensions[LightBird::IImage::TIFF] = ".tiff";
    this->extensions[LightBird::IImage::TGA] = ".tga";
    this->extensions[LightBird::IImage::BMP] = ".bmp";
}

Preview::~Preview()
{
}

QString Preview::generate(const QString &fileId, LightBird::IImage::Format format, unsigned int width, unsigned int height, unsigned int position)
{
    QList<void *>   extensions;
    bool            result = false;
    QTemporaryFile  tmpFile;

    // If the file doesn't exists, we have nothing to do
    if (!this->file.setId(fileId) || !QFileInfo(this->file.getFullPath()).isFile())
        return ("");
    this->width = width;
    this->height = height;
    this->format = format;
    if (position)
        this->position = "_" + QString::number(position);
    // Defines the width and the height of the image
    this->_size();
    // Generates the path and the name of the preview file
    if (this->cacheEnabled)
    {
        this->previewFileName = LightBird::Library::configuration().get("temporaryPath") + "/" + this->file.getId() + "_" + QString::number(this->width) + "x" + QString::number(this->height) + this->position;
        // Search the file in the cache
        if (this->_useCache())
            return (this->previewFileName);
    }
    // If the cache is disabled, the preview is stored in a temporary file
    else
    {
        tmpFile.setFileTemplate(LightBird::Library::configuration().get("temporaryPath") + "/XXXXXX" + this->extensions[format]);
        tmpFile.open();
        tmpFile.setAutoRemove(false);
        this->previewFileName = tmpFile.fileName();
    }
    // Gets the extensions that can generate a preview of the file
    QListIterator<void *> it(extensions = LightBird::Library::extension().get("IPreview"));
    while (it.hasNext() && !result)
        result = static_cast<LightBird::IPreview *>(it.next())->generate(this->file.getFullPath(), this->previewFileName, format, this->width, this->height, position);
    // Release the extensions
    LightBird::Library::extension().release(extensions);
    // No extensions has been able to generate the preview
    if (!result)
        return ("");
    // Moves the preview to the cache directory
    if (this->cacheEnabled)
    {
        SmartMutex mutex(this->mutex);
        // Moves the file
        if (!mutex || !this->_move())
            return ("");
        // Manages the cache
        this->_cache();
    }
    return (this->previewFileName);
}

void    Preview::_size()
{
    if (!this->width && !this->height)
    {
        this->width = 100;
        this->height = 75;
    }
    if (this->width > 1024)
        this->width = 1024;
    if (this->height > 768)
        this->height = 768;
}

bool    Preview::_useCache()
{
    QFileInfo   info;

    // Defines the cache file name
    this->cacheFileName = this->cachePath + "/" + this->file.getId() + "_" + QString::number(this->width) + "x" + QString::number(this->height) + this->position + this->extensions[this->format];
    info.setFile(this->cacheFileName);
    // A preview has been found in the cache
    if (info.isFile())
    {
        // Checks if the preview is not too old
        if (info.lastModified() >= QFileInfo(this->file.getFullPath()).lastModified())
        {
            this->previewFileName = this->cacheFileName;
            return (true);
        }
        else
            QFile::remove(this->cacheFileName);
    }
    return (false);
}

bool    Preview::_move()
{
    // Makes sure that the cache directory exists
    if (!QFileInfo(this->cachePath).isDir() && !QDir().mkpath(this->cachePath))
    {
        LightBird::Library::log().warning("Unable to create the cache directory", Properties("cachePath", this->cachePath).toMap(), "Preview", "_move");
        return (false);
    }
    // Copies the preview to the cache
    if (!LightBird::copy(this->previewFileName, this->cacheFileName))
    {
        LightBird::Library::log().warning("Unable to copy the temporary preview to the cache", Properties("previewFileName", this->previewFileName).add("cacheFileName", this->cacheFileName).toMap(), "Preview", "_move");
        return (false);
    }
    // Removes the temporary preview
    if (!QFile::remove(this->previewFileName))
    {
        LightBird::Library::log().warning("Unable to remove the tmp preview", Properties("previewFileName", this->previewFileName).toMap(), "Preview", "_move");
        return (false);
    }
    this->previewFileName = this->cacheFileName;
    return (true);
}

void    Preview::_cache()
{
    QDir    dir(this->cachePath);
    double  size = 0;

    // If the cache size limit is 0, there is no limit
    if (!this->cacheSizeLimit)
        return ;
    // Removes the oldest files if the cache size is too big
    QStringListIterator it(dir.entryList(QDir::Files, QDir::Time));
    while (it.hasNext())
    {
        if (size > this->cacheSizeLimit && !QFile::remove(this->cachePath + "/" + it.peekNext()))
            LightBird::Library::log().warning("Unable to delete the file", Properties("fileName", this->cachePath + "/" + it.peekNext()).toMap(), "Preview", "_cache");
        size += (((double)QFileInfo(this->cachePath + "/" + it.peekNext()).size()) / (1048576.0));
        it.next();
    }
}
