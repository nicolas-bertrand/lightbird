#include "LightBird.h"
#include "Library.h"
#include "IIdentify.h"

Library::Library(LightBird::IApi &api)
    : ITest(api)
{
}

Library::~Library()
{
}

unsigned int    Library::run()
{
    log.debug("Running the tests of the library...", "Library", "run");
    try
    {
        ASSERT(LightBird::getFileMime("avi") == "video/avi");
        ASSERT(LightBird::getFileMime(".avi") == "video/avi");
        ASSERT(LightBird::getFileMime("video.avi") == "video/avi");
        ASSERT(LightBird::getFileMime("tavi") == "application/octet-stream");
        ASSERT(LightBird::getFileMime(".tavi") == "application/octet-stream");
        ASSERT(LightBird::getFileMime("video.tavi") == "application/octet-stream");
        ASSERT(LightBird::getFileMime("") == "application/octet-stream");

        ASSERT(LightBird::getFileType("avi") == LightBird::IIdentify::VIDEO);
        ASSERT(LightBird::getFileType(".avi") == LightBird::IIdentify::VIDEO);
        ASSERT(LightBird::getFileType("video.avi") == LightBird::IIdentify::VIDEO);
        ASSERT(LightBird::getFileType("tavi") == LightBird::IIdentify::OTHER);
        ASSERT(LightBird::getFileType(".tavi") == LightBird::IIdentify::OTHER);
        ASSERT(LightBird::getFileType("video.tavi") == LightBird::IIdentify::OTHER);
        ASSERT(LightBird::getFileType("") == LightBird::IIdentify::OTHER);

        ASSERT(LightBird::getImageExtension(LightBird::IImage::BMP) == "bmp");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::BMP, false) == "bmp");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::BMP, true) == ".bmp");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::GIF) == "gif");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::JPEG) == "jpeg" || LightBird::getImageExtension(LightBird::IImage::JPEG) == "jpg");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::PNG) == "png");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::TGA) == "tga");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::TIFF) == "tiff");
    }
    catch (unsigned int line)
    {
        this->log.debug("Tests of the library failed!", Properties("line", line).toMap(), "Library", "run");
        return (line);
    }
    this->log.debug("Tests of the library successful!", "Library", "run");
    return (0);
}
