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
}

Preview::~Preview()
{
}

QString Preview::generate(const QString &fileId, LightBird::IImage::Format format, unsigned int width, unsigned int height, unsigned int position)
{
    QList<void *>   extensions;
    Parameters      params;
    bool            result = false;
    QTemporaryFile  tmpFile;

    // If the file doesn't exists, we have nothing to do
    if (!params.file.setId(fileId) || !QFileInfo(params.file.getFullPath()).isFile())
        return ("");
    params.width = width;
    params.height = height;
    params.format = format;
    params.extension = LightBird::getImageExtension(format, true);
    if (position)
        params.position = "_" + QString::number(position);
    // Defines the width and the height of the image
    this->_size(params);
    // Generates the path and the name of the preview file
    if (this->cacheEnabled)
    {
        params.previewFileName = LightBird::Library::configuration().get("temporaryPath") + "/" + params.file.getId() + "_" + QString::number(params.width) + "x" + QString::number(params.height) + params.position;
        // Search the file in the cache
        if (this->_useCache(params))
            return (params.previewFileName);
    }
    // If the cache is disabled, the preview is stored in a temporary file
    else
    {
        tmpFile.setFileTemplate(LightBird::Library::configuration().get("temporaryPath") + "/XXXXXX" + params.extension);
        tmpFile.open();
        tmpFile.setAutoRemove(false);
        params.previewFileName = tmpFile.fileName();
        params.previewFileName.chop(params.extension.size());
    }
    // Gets the extensions that can generate a preview of the file
    QListIterator<void *> it(extensions = LightBird::Library::extension().get("IPreview"));
    while (it.hasNext() && !result)
        result = static_cast<LightBird::IPreview *>(it.next())->generate(params.file.getFullPath(), params.previewFileName, format, params.width, params.height, position);
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
        if (!mutex || !this->_move(params))
            return ("");
        // Manages the cache
        this->_cache();
    }
    return (params.previewFileName);
}

void    Preview::_size(Parameters &params)
{
    if (!params.width && !params.height)
    {
        params.width = 100;
        params.height = 75;
    }
    if (params.width > 1024)
        params.width = 1024;
    if (params.height > 768)
        params.height = 768;
}

bool    Preview::_useCache(Parameters &params)
{
    QFileInfo   info;

    // Defines the cache file name
    params.cacheFileName = this->cachePath + "/" + params.file.getId() + "_" + QString::number(params.width) + "x" + QString::number(params.height) + params.position + params.extension;
    info.setFile(params.cacheFileName);
    // A preview has been found in the cache
    if (info.isFile())
    {
        // Checks if the preview is not too old
        if (info.lastModified() >= QFileInfo(params.file.getFullPath()).lastModified())
        {
            params.previewFileName = params.cacheFileName;
            return (true);
        }
        else
            QFile::remove(params.cacheFileName);
    }
    return (false);
}

bool    Preview::_move(Parameters &params)
{
    // Makes sure that the cache directory exists
    if (!QFileInfo(this->cachePath).isDir() && !QDir().mkpath(this->cachePath))
    {
        LightBird::Library::log().warning("Unable to create the cache directory", Properties("cachePath", this->cachePath).toMap(), "Preview", "_move");
        return (false);
    }
    // Copies the preview to the cache
    if (!LightBird::copy(params.previewFileName, params.cacheFileName))
    {
        LightBird::Library::log().warning("Unable to copy the temporary preview to the cache", Properties("previewFileName", params.previewFileName).add("cacheFileName", params.cacheFileName).toMap(), "Preview", "_move");
        return (false);
    }
    // Removes the temporary preview
    if (!QFile::remove(params.previewFileName))
    {
        LightBird::Library::log().warning("Unable to remove the tmp preview", Properties("previewFileName", params.previewFileName).toMap(), "Preview", "_move");
        return (false);
    }
    params.previewFileName = params.cacheFileName;
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
