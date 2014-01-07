#include <QFileInfo>
#include <QTemporaryFile>

#include "Identify.h"
#include "LightBird.h"

Identify::Identify(LightBird::IApi *a)
    : api(a)
{
    this->isInitialized();

    this->colorSpace.insert(MagickCore::UndefinedColorspace, "Undefined");
    this->colorSpace.insert(MagickCore::LabColorspace, "CIELab");
    this->colorSpace.insert(MagickCore::CMYColorspace, "CMY");
    this->colorSpace.insert(MagickCore::CMYKColorspace, "CMYK");
    this->colorSpace.insert(MagickCore::GRAYColorspace, "Gray");
    this->colorSpace.insert(MagickCore::HCLColorspace, "HCL");
    this->colorSpace.insert(MagickCore::HSBColorspace, "HSB");
    this->colorSpace.insert(MagickCore::HSLColorspace, "HSL");
    this->colorSpace.insert(MagickCore::HWBColorspace, "HWB");
    this->colorSpace.insert(MagickCore::LabColorspace, "Lab");
    this->colorSpace.insert(MagickCore::LCHColorspace, "LCH");
    this->colorSpace.insert(MagickCore::LMSColorspace, "LMS");
    this->colorSpace.insert(MagickCore::LogColorspace, "Log");
    this->colorSpace.insert(MagickCore::LuvColorspace, "Luv");
    this->colorSpace.insert(MagickCore::OHTAColorspace, "OHTA");
    this->colorSpace.insert(MagickCore::Rec601YCbCrColorspace, "Rec601YCbCr");
    this->colorSpace.insert(MagickCore::Rec709YCbCrColorspace, "Rec709YCbCr");
    this->colorSpace.insert(MagickCore::RGBColorspace, "RGB");
    this->colorSpace.insert(MagickCore::sRGBColorspace, "sRGB");
    this->colorSpace.insert(MagickCore::TransparentColorspace, "Transparent");
    this->colorSpace.insert(MagickCore::XYZColorspace, "XYZ");
    this->colorSpace.insert(MagickCore::YCbCrColorspace, "YCbCr");
    this->colorSpace.insert(MagickCore::YCCColorspace, "YCC");
    this->colorSpace.insert(MagickCore::YIQColorspace, "YIQ");
    this->colorSpace.insert(MagickCore::YPbPrColorspace, "YPbPr");
    this->colorSpace.insert(MagickCore::YUVColorspace, "YUV");

    this->compression.insert(MagickCore::UndefinedCompression, "Undefined");
    this->compression.insert(MagickCore::B44Compression, "B44");
    this->compression.insert(MagickCore::B44ACompression, "B44A");
    this->compression.insert(MagickCore::BZipCompression, "BZip");
    this->compression.insert(MagickCore::DXT1Compression, "DXT1");
    this->compression.insert(MagickCore::DXT3Compression, "DXT3");
    this->compression.insert(MagickCore::DXT5Compression, "DXT5");
    this->compression.insert(MagickCore::FaxCompression, "Fax");
    this->compression.insert(MagickCore::Group4Compression, "Group4");
    this->compression.insert(MagickCore::JBIG1Compression, "JBIG1");
    this->compression.insert(MagickCore::JBIG2Compression, "JBIG2");
    this->compression.insert(MagickCore::JPEGCompression, "JPEG");
    this->compression.insert(MagickCore::JPEG2000Compression, "JPEG2000");
    this->compression.insert(MagickCore::LosslessJPEGCompression, "Lossless");
    this->compression.insert(MagickCore::LosslessJPEGCompression, "LosslessJPEG");
    this->compression.insert(MagickCore::LZMACompression, "LZMA");
    this->compression.insert(MagickCore::LZWCompression, "LZW");
    this->compression.insert(MagickCore::NoCompression, "None");
    this->compression.insert(MagickCore::PizCompression, "Piz");
    this->compression.insert(MagickCore::Pxr24Compression, "Pxr24");
    this->compression.insert(MagickCore::RLECompression, "RLE");
    this->compression.insert(MagickCore::ZipCompression, "Zip");
    this->compression.insert(MagickCore::RLECompression, "RunlengthEncoded");
    this->compression.insert(MagickCore::ZipSCompression, "ZipS");
}

Identify::~Identify()
{
}

bool    Identify::identify(const QString &file, LightBird::IIdentify::Information &information)
{
    // Check that the file exists
    if (!QFileInfo(file).isFile())
        return (false);
    information.data.clear();
    information.type = LightBird::IIdentify::OTHER;

    try
    {
        Magick::Image image(file.toStdString());
        information.data.insert("format", QString::fromStdString(image.magick()));
        information.data.insert("width", QString::number(image.columns()));
        information.data.insert("height", QString::number(image.rows()));
        information.data.insert("red depth", QString::number(image.channelDepth(Magick::RedChannel)) + "-bit");
        information.data.insert("blue depth", QString::number(image.channelDepth(Magick::BlueChannel)) + "-bit");
        information.data.insert("green depth", QString::number(image.channelDepth(Magick::GreenChannel)) + "-bit");
        information.data.insert("depth", QString::number(image.depth()) + "-bit");
        information.data.insert("quality", QString::number(image.quality()));
        information.data.insert("colorspace", colorSpace.value(image.colorSpace()));
        information.data.insert("compression", compression.value(image.compressType()));
        information.data.insert("gamma", QString::number(image.gamma()));
    }
    catch (Magick::Exception &e)
    {
        LOG_DEBUG("An error occured while identifying an image, using ImageMagick", Properties("what", e.what()).add("image", file).toMap(), "Identify", "identify");
        return (false);
    }

    if (information.data.isEmpty())
        return (false);
    information.type = LightBird::IIdentify::IMAGE;
    return (true);
}
