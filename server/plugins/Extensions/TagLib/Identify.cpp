#include "Identify.h"

#include <QFileInfo>
#include <tag.h>
#include <taglib.h>
#include <fileref.h>
#include <tstring.h>
#include <tbytevector.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <id3v2header.h>
#include <id3v1tag.h>
#include <attachedpictureframe.h>
#include <apetag.h>

Identify::Identify(LightBird::IApi *a)
    : _api(a)
{
    _types << LightBird::IIdentify::AUDIO;

    _id3v2["TALB"] = "album";
    _id3v2["TIT2"] = "title";
    _id3v2["TLAN"] = "language";
    _id3v2["TCOM"] = "composer";
    _id3v2["TCON"] = "genre";
    _id3v2["TRCK"] = "track";
    _id3v2["TPE2"] = "artist";
}

Identify::~Identify()
{
}

bool    Identify::identify(const QString &fileName, LightBird::IIdentify::Information &information)
{
    // Checks that the file exists
    if (!QFileInfo(fileName).isFile())
        return (false);
    information.data.clear();
    information.type = LightBird::IIdentify::OTHER;
    // Gets The meta data of the file
    TagLib::FileRef file(fileName.toLatin1().data());
    if (!file.isNull())
        this->_addTags(information, file);
    TagLib::MPEG::File mpeg(fileName.toLatin1().data());
    if (mpeg.isOpen())
    {
        this->_addApe(information, mpeg);
        this->_addID3v1Data(information, mpeg);
        this->_addID3v2Data(information, mpeg);
    }
    if (!mpeg.isOpen() && file.isNull())
        return (false);
    return (true);
}

void    Identify::_addTags(LightBird::IIdentify::Information &information, TagLib::FileRef &file)
{
    TagLib::Tag *tag;
    TagLib::AudioProperties *properties;

    if (file.isNull())
        return ;
    if(file.tag())
    {
        tag = file.tag();
        this->_addData(information, "title", tag->title().toCString());
        this->_addData(information, "artist", tag->artist().toCString());
        this->_addData(information, "album", tag->album().toCString());
        this->_addData(information, "year", tag->year());
        this->_addData(information, "track", tag->track());
        this->_addData(information, "genre", tag->genre().toCString());
    }
    if(file.audioProperties())
    {
        properties = file.audioProperties();
        this->_addData(information, "bit rate", properties->bitrate() * 1000);
        this->_addData(information, "sample rate", properties->sampleRate());
        this->_addData(information, "channels", properties->channels());
        this->_addData(information, "duration", properties->length());
    }
}

void    Identify::_addID3v2Data(LightBird::IIdentify::Information &information, TagLib::MPEG::File &file)
{
    TagLib::ID3v2::Tag *id3v2tag = file.ID3v2Tag();
    TagLib::ID3v2::FrameList::ConstIterator it;
    if(!id3v2tag)
        return ;
    for(it = id3v2tag->frameList().begin(); it != id3v2tag->frameList().end(); it++)
        if (this->_id3v2.contains((*it)->frameID().data()))
            this->_addData(information, this->_id3v2[(*it)->frameID().data()], (*it)->toString().toCString());
}

void    Identify::_addID3v1Data(LightBird::IIdentify::Information &information, TagLib::MPEG::File &file)
{
    TagLib::ID3v1::Tag *id3v1tag = file.ID3v1Tag();

    if(!id3v1tag)
        return ;
    this->_addData(information, "title", id3v1tag->title().toCString());
    this->_addData(information, "artist", id3v1tag->artist().toCString());
    this->_addData(information, "album", id3v1tag->album().toCString());
    this->_addData(information, "year", id3v1tag->year());
    this->_addData(information, "track", id3v1tag->track());
    this->_addData(information, "genre", id3v1tag->genre().toCString());
}

void    Identify::_addApe(LightBird::IIdentify::Information &information, TagLib::MPEG::File &file)
{
    TagLib::APE::Tag *ape = file.APETag();
    TagLib::APE::ItemListMap::ConstIterator it;
    if(!ape)
        return ;
    for(it = ape->itemListMap().begin(); it != ape->itemListMap().end(); ++it)
        this->_addData(information, (*it).first.toCString(), (*it).second.toString().toCString());
}

void    Identify::_addData(LightBird::IIdentify::Information &information, const QString &key, const QVariant &value)
{
    if (!key.trimmed().isEmpty() && !value.toString().trimmed().isEmpty() && value.toString() != "0")
        information.data.insert(key, value);
}
