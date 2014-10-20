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
        ASSERT(LightBird::getFileMime("AVI") == "video/avi");
        ASSERT(LightBird::getFileMime("Avi") == "video/avi");
        ASSERT(LightBird::getFileMime("AvI") == "video/avi");

        ASSERT(LightBird::getFileType("avi") == LightBird::IIdentify::VIDEO);
        ASSERT(LightBird::getFileType(".avi") == LightBird::IIdentify::VIDEO);
        ASSERT(LightBird::getFileType("video.avi") == LightBird::IIdentify::VIDEO);
        ASSERT(LightBird::getFileType("tavi") == LightBird::IIdentify::OTHER);
        ASSERT(LightBird::getFileType(".tavi") == LightBird::IIdentify::OTHER);
        ASSERT(LightBird::getFileType("video.tavi") == LightBird::IIdentify::OTHER);
        ASSERT(LightBird::getFileType("") == LightBird::IIdentify::OTHER);
        ASSERT(LightBird::getFileType("AVI") == LightBird::IIdentify::VIDEO);
        ASSERT(LightBird::getFileType("Avi") == LightBird::IIdentify::VIDEO);
        ASSERT(LightBird::getFileType("AvI") == LightBird::IIdentify::VIDEO);

        ASSERT(LightBird::getImageExtension(LightBird::IImage::BMP) == "bmp");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::BMP, false) == "bmp");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::BMP, true) == ".bmp");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::GIF) == "gif");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::JPEG) == "jpeg" || LightBird::getImageExtension(LightBird::IImage::JPEG) == "jpg");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::PNG) == "png");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::TGA) == "tga");
        ASSERT(LightBird::getImageExtension(LightBird::IImage::TIFF) == "tiff");

        ASSERT(LightBird::stringToBytes("1") == 1);
        ASSERT(LightBird::stringToBytes("15489489") == 15489489);
        ASSERT(LightBird::stringToBytes("1K") == 1024);
        ASSERT(LightBird::stringToBytes("1565k") == 1565 * 1024);
        ASSERT(LightBird::stringToBytes("1 k") == 1024);
        ASSERT(LightBird::stringToBytes("1  k ") == 1024);
        ASSERT(LightBird::stringToBytes("  1   K  ") == 1024);
        ASSERT(LightBird::stringToBytes("  3   K  ") == 3 * 1024);
        ASSERT(LightBird::stringToBytes("1M") == 1024 * 1024);
        ASSERT(LightBird::stringToBytes("1g") == 1024 * 1024 * 1024);
        ASSERT(LightBird::stringToBytes("5t") == quint64(5) * 1024 * 1024 * 1024 * 1024);
    }
    catch (unsigned int line)
    {
        this->log.debug("Tests of the library failed!", Properties("line", line).toMap(), "Library", "run");
        return (line);
    }
    this->log.debug("Tests of the library successful!", "Library", "run");
    return (0);
}
