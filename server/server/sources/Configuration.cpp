#include <QDir>

#include "Configuration.h"
#include "Configurations.h"
#include "Defines.h"
#include "Events.h"
#include "LightBird.h"
#include "Log.h"
#include "Mutex.h"

Configuration::Configuration(const QString &configurationPath, const QString &alternativePath, QObject *parent)
    : QObject(parent)
{
    this->_load(configurationPath, alternativePath);
    QObject::connect(this, SIGNAL(setParentSignal(QObject*)), this, SLOT(_setParent(QObject*)), Qt::QueuedConnection);
}

Configuration::Configuration()
{
    QObject::connect(this, SIGNAL(setParentSignal(QObject*)), this, SLOT(_setParent(QObject*)), Qt::QueuedConnection);
}

Configuration::~Configuration()
{
    if (!this->file.fileName().isEmpty())
        LOG_TRACE("Configuration destroyed!", Properties("file", this->file.fileName()), "Configuration", "~Configuration");
}

bool    Configuration::_load(const QString &configurationPath, const QString &alternativePath)
{
    QDir    directory;
    QString dirName;
    QString errorMsg;
    QString configuration;
    QString alternative;
    int     errorLine;
    int     errorColumn;

    // Make sure that the path separators is /
    configuration = configurationPath;
    configuration.replace('\\', '/');
    alternative = alternativePath;
    alternative.replace('\\', '/');
    // If the configurationPath is empty, we can't load it
    if (configuration.isEmpty())
    {
        LOG_ERROR("The configuration path is not valid", Properties("file", configurationPath), "Configuration", "_load");
        return (false);
    }
    // If the file is not defined after the directories, we add the defaut configuration file name
    if (configuration.at(configuration.size() - 1) == '/')
        configuration += DEFAULT_CONFIGURATION_FILE;
    // Get the directory of the configuration if their is one
    if (configuration.contains('/'))
        dirName = configuration.left(configuration.lastIndexOf('/'));
    // If configurationPath does not exist we try to create it using the alternativePath
    this->file.setFileName(configuration);
    if (!QFileInfo(this->file.fileName()).isFile())
    {
        LOG_DEBUG("The configuration file does not exist and will be created using the alternative file", Properties("configuration", this->file.fileName()).add("alternative", alternative), "Configuration", "_load");
        if (!QFileInfo(alternative).isFile())
        {
            LOG_WARNING("The alternative path does not exist either. The configuration can't be loaded.", Properties("configuration", this->file.fileName()).add("alternative", alternative), "Configuration", "_load");
            return (false);
        }
        // Creates the directory of the configuration file if it does not exist
        if (!dirName.isEmpty() && !directory.exists(dirName) && !directory.mkpath(dirName))
        {
            LOG_ERROR("Cannot creates the directory of the configuration", Properties("directory", dirName), "Configuration", "_load");
            return (false);
        }
        // Creates the configuration file using the alternative file
        if (LightBird::copy(alternative, this->file.fileName()) == false)
        {
            LOG_ERROR("Cannot creates the configuration file from the alternative file", Properties("configuration", this->file.fileName()).add("alternative", alternative), "Configuration", "_load");
            return (false);
        }
    }
    // Ensure that the permission of the file are correct
    //if (this->file.setPermissions(QFile::ReadUser | QFile::WriteUser))
    //  LOG_ERROR("The permissions of the configuration file cannot be modified", "Configuration", "_load");
    // Open the configuration file
    if (!this->file.open(QIODevice::ReadOnly))
    {
        LOG_ERROR("Cannot open the configuration file", Properties("file", this->file.fileName()), "Configuration", "_load");
        return (false);
    }
    // Parse the configuration file into a DOM representation
    if (!this->doc.setContent(&this->file, false, &errorMsg, &errorLine, &errorColumn))
    {
        LOG_ERROR("An error occured while parsing the configuration file", Properties("message", errorMsg).add("file", file.fileName())
                  .add("line", QString::number(errorLine)).add("column", errorColumn), "Configuration", "_load");
        return (false);
    }
    LOG_DEBUG("Configuration loaded", Properties("file", this->file.fileName()), "Configuration", "_load");
    this->file.close();
    this->dom = this->doc.documentElement();
    this->isInitialized();
    return (true);
}

QString Configuration::getPath() const
{
    return (this->file.fileName());
}

QString Configuration::get(const QString &nodeName) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Configuration", "get");

    if (!mutex)
        return ("");
    return (this->_get(nodeName, this->dom));
}

unsigned int    Configuration::count(const QString &nodeName) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Configuration", "count");

    if (!mutex)
        return (0);
    return (this->_count(nodeName, this->dom));
}

LightBird::IConfiguration &Configuration::set(const QString &nodeName, const QString &nodeValue)
{
    Mutex   mutex(this->mutex, "Configuration", "set");

    if (mutex)
        this->_set(nodeName, nodeValue, this->dom);
    return (*this);
}

bool    Configuration::remove(const QString &nodeName)
{
    Mutex   mutex(this->mutex, "Configuration", "remove");

    if (!mutex)
        return (false);
    return (this->_remove(nodeName, this->dom));
}

QDomElement Configuration::readDom() const
{
    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
        LOG_ERROR("Deadlock", "Configuration", "readDom");
    return (this->dom);
}

QDomElement Configuration::writeDom()
{
    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
        LOG_ERROR("Deadlock", "Configuration", "writeDom");
    return (this->dom);
}

void    Configuration::release() const
{
    this->mutex.unlock();
}

bool    Configuration::save()
{
    Mutex       mutex(this->mutex, "Configuration", "save");
    QByteArray  data;
    int         wrote;

    if (!mutex)
        return (false);
    if (*this == false)
        return (false);
    if (this->file.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
    {
        LOG_ERROR("Cannot open the configuration file in order to save the configuration", Properties("file",  this->file.fileName()), "Configuration", "save");
        return (false);
    }
    data = this->doc.toByteArray(2);
    if ((wrote = this->file.write(data)) != data.size())
    {
        LOG_ERROR("Unable to write all the data in the configuration file", Properties("file",  this->file.fileName())
                  .add("data", data).add("wrote", wrote), "Configuration", "save");
        this->file.close();
        return (false);
    }
    this->file.close();
    // If the configuration of the server has been saved, an event occured
    if (Configurations::instance() == this)
        Events::instance()->send("configuration_saved");
    LOG_DEBUG("Configuration saved", Properties("file", this->file.fileName()), "Configuration", "save");
    return (true);
}

QString Configuration::_get(const QString &nodeName, QDomElement element) const
{
    QString  result = "";
    QDomNode node;
    QString  name;
    QString  attribut;
    QString  tmp;
    int      index;

    if (*this == false)
        return ("");
    QStringListIterator it(nodeName.split('/'));
    while (it.hasNext() == true && element.isNull() == false)
    {
        index = 0;
        name = it.peekNext();
        attribut.clear();
        if (it.peekNext().contains('.') == true)
        {
            name = it.peekNext().left(it.peekNext().lastIndexOf('.'));
            attribut = it.peekNext().right(it.peekNext().size() - name.size() - 1);
        }
        if (name.right(1) == "]" && name.contains('[') == true)
        {
            tmp = name.right(name.size() - name.lastIndexOf('[') - 1);
            tmp.resize(tmp.size() - 1);
            index = tmp.toInt();
            name.resize(name.lastIndexOf('['));
        }
        element = element.firstChildElement(name).toElement();
        if (element.isNull() == true)
            Log::trace("Node not found in the configuration", Properties("nodeName", nodeName).add("file", this->file.fileName()), "Configuration", "get");
        else if (index > 0)
            while ((element = element.nextSiblingElement(name)).isNull() == false && --index > 0)
                ;
        it.next();
        if (it.hasNext() == false)
        {
            if (attribut.isEmpty() == false)
                result = element.attribute(attribut);
            else
            {
                element.normalize();
                for (node = element.firstChild(); node.isNull() == false; node = node.nextSibling())
                    if (node.isText() == true)
                        result += node.nodeValue();
            }
        }
    }
    return (result);
}

unsigned int    Configuration::_count(const QString &nodeName, QDomElement element) const
{
    unsigned int result = 0;
    QString      name;
    QString      tmp;
    int          index;
    int          i;

    if (*this == false)
        return (0);
    if (nodeName.isEmpty())
        return (0);
    i = nodeName.count('/') + 1;
    QStringListIterator it(nodeName.split('/'));
    while (it.hasNext() == true && element.isNull() == false)
    {
        --i;
        index = 0;
        name = it.peekNext();
        if (name.right(1) == "]" && name.contains('[') == true)
        {
            tmp = name.right(name.size() - name.lastIndexOf('[') - 1);
            tmp.resize(tmp.size() - 1);
            index = tmp.toInt();
            name.resize(name.lastIndexOf('['));
        }
        element = element.firstChildElement(name).toElement();
        if (i <= 0 && element.isNull() == false && ++result)
            while ((element = element.nextSiblingElement(it.peekNext())).isNull() == false)
                ++result;
        if (!element.isNull() && index > 0)
            while ((element = element.nextSiblingElement(name)).isNull() == false && --index > 0)
                ;
        it.next();
    }
    return (result);
}

void    Configuration::_set(const QString &nodeName, const QString &nodeValue, QDomElement element)
{
    QDomText    text;
    QDomElement newElement;
    QDomNode    node;
    QString     name;
    QString     attribut;
    QString     tmp;
    int         index;

    if (*this == false || element.isNull())
        return ;
    QStringListIterator it(nodeName.split('/'));
    while (it.hasNext() == true)
    {
        index = 0;
        name = it.peekNext();
        attribut.clear();
        if (it.peekNext().contains('.') == true)
        {
            name = it.peekNext().left(it.peekNext().lastIndexOf('.'));
            attribut = it.peekNext().right(it.peekNext().size() - name.size() - 1);
        }
        if (name.right(1) == "]" && name.contains('[') == true)
        {
            tmp = name.right(name.size() - name.lastIndexOf('[') - 1);
            tmp.resize(tmp.size() - 1);
            index = tmp.toInt();
            name.resize(name.lastIndexOf('['));
        }
        if (element.firstChildElement(name).isNull() == true)
        {
            newElement = this->doc.createElement(name);
            element.appendChild(newElement);
            element = element.firstChildElement(name).toElement();
        }
        else
        {
            element = element.firstChildElement(name).toElement();
            while (element.nextSiblingElement(name).isNull() == false && index-- > 0)
                element = element.nextSiblingElement(name);
            if (index > 0)
            {
                newElement = this->doc.createElement(name);
                element = element.parentNode().appendChild(newElement).toElement();
            }
        }
        it.next();
        if (it.hasNext() == false)
        {
            if (attribut.isEmpty() == false)
                element.setAttribute(attribut, nodeValue);
            else if (element.firstChild().isNull() == true)
            {
                text = this->doc.createTextNode(nodeValue);
                element.appendChild(text);
            }
            else
            {
                element.normalize();
                node = element.firstChild();
                while (node.isNull() == false)
                {
                    if (node.isText())
                    {
                        node.setNodeValue(nodeValue);
                        break;
                    }
                    else if (node.nextSibling().isNull() == true)
                    {
                        text = this->doc.createTextNode(nodeValue);
                        element.appendChild(text);
                    }
                    node = node.nextSibling();
                }
            }
        }
    }
}

bool    Configuration::_remove(const QString &nodeName, QDomElement element)
{
    bool    result = false;
    QString name;
    QString tmp;
    int     index;
    int     i;

    if (*this == false)
        return (false);
    if (nodeName.isEmpty())
        return (false);
    i = nodeName.count('/') + 1;
    QStringListIterator it(nodeName.split('/'));
    while (it.hasNext() == true && element.isNull() == false && !result)
    {
        --i;
        index = 0;
        name = it.peekNext();
        if (name.right(1) == "]" && name.contains('[') == true)
        {
            tmp = name.right(name.size() - name.lastIndexOf('[') - 1);
            tmp.resize(tmp.size() - 1);
            index = tmp.toInt();
            name.resize(name.lastIndexOf('['));
        }
        element = element.firstChildElement(name).toElement();
        if (i <= 0 && element.isNull() == false)
            while (!element.isNull() && index >= 0 && !result)
            {
                if (index-- <= 0)
                {
                    element.parentNode().removeChild(element);
                    result = true;
                }
                element = element.nextSiblingElement(name);
            }
        if (!element.isNull() && index > 0 && !result)
            while ((element = element.nextSiblingElement(name)).isNull() == false && --index > 0)
                ;
        it.next();
    }
    return (result);
}

void    Configuration::setParent(QObject *parent)
{
    emit this->setParentSignal(parent);
}

void    Configuration::_setParent(QObject *parent)
{
    QObject::setParent(parent);
}
