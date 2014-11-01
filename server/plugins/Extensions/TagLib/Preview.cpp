#include <QFileInfo>
#include <QTemporaryFile>

#include "LightBird.h"
#include "Preview.h"

#include <tag.h>
#include <taglib.h>
#include <fileref.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <id3v2header.h>
#include <attachedpictureframe.h>

Preview::Preview(LightBird::IApi *a)
    : api(a)
{
    this->fileTemplate = LightBird::c().temporaryPath + "/" + "XXXXXX.jpeg";
}

Preview::~Preview()
{
}

bool    Preview::generate(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width, unsigned int height, unsigned int, float quality)
{
    QImage                  image;
    QTemporaryFile          tmp;
    QList<void *>           extensions;
    bool                    result = false;

    // Checks that the source exists
    if (!QFileInfo(source).isFile() || source == destination || destination.isEmpty())
        return (false);
    // Gets the image of the music if there is one
    TagLib::MPEG::File f(source.toLatin1().data());
    TagLib::ID3v2::Tag *id3v2tag = f.ID3v2Tag();
    if(!id3v2tag)
        return (false);
    TagLib::ID3v2::FrameList l = f.ID3v2Tag()->frameListMap()["APIC"];
    if (l.isEmpty())
        return (false);
    TagLib::ID3v2::AttachedPictureFrame *pic = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(l.front());
    image.loadFromData((const uchar *)pic->picture().data(), (int)pic->picture().size());
    // Tries to save the image using Qt
    if (LightBird::saveImage(image, destination, (format ? format : LightBird::c().preview.defaultFormat), quality))
        return (true);
    // Otherwise converts the jpeg file generated into the requested format using the IImage extension
    tmp.setFileTemplate(this->fileTemplate);
    if (!tmp.open())
    {
        LOG_ERROR("Error with QTemporaryFile::open", Properties("file name", tmp.fileName()).toMap(), "Preview", "generate");
        return (false);
    }
    if (!image.save(tmp.fileName()))
        return (false);
    QListIterator<void *> it(extensions = this->api->extensions().get("IImage"));
    while (it.hasNext() && !result)
        result = static_cast<LightBird::IImage *>(it.next())->convert(tmp.fileName(), destination, format, width, height, quality);
    this->api->extensions().release(extensions);
    return (result);
}
