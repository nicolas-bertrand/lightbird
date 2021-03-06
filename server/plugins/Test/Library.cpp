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
    _log.debug("Running the tests of the library...", "Library", "run");
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
        ASSERT(LightBird::getImageExtension(LightBird::IImage::NONE) == "");

        ASSERT(LightBird::getImageFormat("bmp") == LightBird::IImage::BMP);
        ASSERT(LightBird::getImageFormat("BMP") == LightBird::IImage::BMP);
        ASSERT(LightBird::getImageFormat("BmP") == LightBird::IImage::BMP);
        ASSERT(LightBird::getImageFormat(".BmP") == LightBird::IImage::BMP);
        ASSERT(LightBird::getImageFormat("file.BmP") == LightBird::IImage::BMP);
        ASSERT(LightBird::getImageFormat("./folder/file.BmP") == LightBird::IImage::BMP);
        ASSERT(LightBird::getImageFormat("gif") == LightBird::IImage::GIF);
        ASSERT(LightBird::getImageFormat("jpeg") == LightBird::IImage::JPEG);
        ASSERT(LightBird::getImageFormat("jpg") == LightBird::IImage::JPEG);
        ASSERT(LightBird::getImageFormat(".jPg") == LightBird::IImage::JPEG);
        ASSERT(LightBird::getImageFormat("jpg.") == LightBird::IImage::NONE);
        ASSERT(LightBird::getImageFormat(".jPg", LightBird::IImage::BMP) == LightBird::IImage::JPEG);
        ASSERT(LightBird::getImageFormat("png") == LightBird::IImage::PNG);
        ASSERT(LightBird::getImageFormat("tga") == LightBird::IImage::TGA);
        ASSERT(LightBird::getImageFormat("tiff") == LightBird::IImage::TIFF);
        ASSERT(LightBird::getImageFormat("mpeg") == LightBird::IImage::NONE);
        ASSERT(LightBird::getImageFormat("mpeg", LightBird::IImage::JPEG) == LightBird::IImage::JPEG);

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
        _log.debug("Tests of the library failed!", Properties("line", line).toMap(), "Library", "run");
        return (line);
    }
    _log.debug("Tests of the library successful!", "Library", "run");
    return (0);
}
