#include <QCryptographicHash>
#include <QFileInfo>

#include "Identifier.h"

Identifier::Identifier(LightBird::IApi &a) : api(a)
{
    this->mimeDocument.push_back("text/");
    this->mimeDocument.push_back("pdf");
    this->mimeDocument.push_back("excel");
    this->mimeDocument.push_back("msword");
    this->mimeDocument.push_back("powerpoint");
    this->mimeDocument.push_back("opendocument");
    this->mimeDocument.push_back("postscript");
    this->mimeDocument.push_back("xml");
    this->mimeDocument.push_back("json");
    this->maxSizeHash = api.configuration(true).get("maxSizeHash").toLongLong();
}

Identifier::~Identifier()
{
}

LightBird::IIdentify::Information Identifier::identify(const QString &file)
{
    Info            result;
    Info            tmp;
    QList<void *>   extentions;
    QMap<LightBird::IIdentify::Type, QVariantMap> info;

    QTime t;
    t.start();
    result.type = LightBird::IIdentify::OTHER;
    // Check that the file exists
    if (!QFileInfo(file).isFile())
        return (result);
    // Get the extensions that implements IIdentify
    QListIterator<void *> it(extentions = this->api.extensions().get("IIdentify"));
    while (it.hasNext())
    {
        tmp.data.clear();
        tmp.type = LightBird::IIdentify::OTHER;
        // If the plugin could identify the file, add it to the list
        if (static_cast<LightBird::IIdentify *>(it.peekNext())->identify(file, tmp))
            info.insertMulti(tmp.type, tmp.data);
        it.next();
    }
    // Release the extensions
    this->api.extensions().release(extentions);
    // Put the data gathered in the result
    if (info.size())
        this->_identify(info, result);
    // Get the size and the extension of the file
    result.data.insert("size", QFileInfo(file).size());
    if (file.contains("."))
    {
        result.data.insert("extension", file.right(file.size() - file.lastIndexOf(".") - 1));
        result.data.insert("mime", this->getMime(file));
    }
    // Determine if the file is a document
    if (result.type == LightBird::IIdentify::OTHER)
        this->_document(result);
    // Compute the hashes of the file
    this->_hash(file, result);
    // Debug
    /**this->api.log().debug("Type: " + QString::number(result.type));
    std::cout << "Type: " << QString::number(result.type).toStdString() << std::endl;
    QMapIterator<QString, QVariant> i(result.data);
    while (i.hasNext())
    {
        i.next();
        this->api.log().debug(i.key() + QString(17 - i.key().size(), ' ') + "=  " + i.value().toString());
    }//*/
    return (result);
}

QString     Identifier::getMime(const QString &file)
{
    QString                     extension;
    LightBird::IConfiguration   *configuration = NULL;

    if (file.contains(".") && (configuration = this->api.configuration(this->api.getPluginPath() + "/" + "Mime.xml")))
    {
        extension = file.right(file.size() - file.lastIndexOf(".") - 1);
        if (!(extension = configuration->get(extension.toLower())).isEmpty())
            return (extension);
    }
    return ("application/octet-stream");
}

void    Identifier::_identify(QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result)
{
    if (this->_add(LightBird::IIdentify::DOCUMENT, info, result))
        return ;
    if (this->_add(LightBird::IIdentify::IMAGE, info, result))
        return ;
    if (this->_add(LightBird::IIdentify::VIDEO, info, result))
        return ;
    if (this->_add(LightBird::IIdentify::AUDIO, info, result))
        return ;
}

bool    Identifier::_add(LightBird::IIdentify::Type type, QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result)
{
    if (info.contains(type))
    {
        QListIterator<QVariantMap> i(info.values(type));
        while (i.hasNext())
        {
            result.data = i.peekNext();
            i.next();
        }
        if (!result.data.isEmpty() || type == LightBird::IIdentify::DOCUMENT)
        {
            result.type = type;
            return (true);
        }
    }
    return (false);
}

void    Identifier::_document(Info &result)
{
    QString mime = result.data.value("mime").toString();

    QStringListIterator it(this->mimeDocument);
    while (it.hasNext())
        if (mime.contains(it.next()))
        {
            result.type = LightBird::IIdentify::DOCUMENT;
            return ;
        }
}

void    Identifier::_hash(const QString &fileName, Info &result)
{
    QCryptographicHash  md5(QCryptographicHash::Md5);
    //QCryptographicHash  sha1(QCryptographicHash::Sha1);
    QFile               file(fileName);
    QByteArray          data;

    if ((this->maxSizeHash >= 0 && (this->maxSizeHash == 0 || this->maxSizeHash < QFileInfo(fileName).size()))
        || !file.open(QIODevice::ReadOnly))
        return ;
    do
    {
        data.clear();
        data = file.read(READ_FILE_SIZE);
        md5.addData(data);
        //sha1.addData(data);
    }
    while (data.size() == READ_FILE_SIZE);
    result.data.insert("md5", md5.result().toHex());
    //result.data.insert("sha1", sha1.result().toHex());
}
