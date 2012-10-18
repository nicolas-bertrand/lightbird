#include <QFile>
#include <QFileInfo>

#include "ApiConfiguration.h"
#include "Configurations.h"
#include "Log.h"
#include "Plugins.hpp"

ApiConfiguration::ApiConfiguration(const QString &idPlugin) : id(Plugins::checkId(idPlugin)),
                                                              configuration(*Configurations::instance())
{
    // The plugin doesn't exists
    if (!QFileInfo(Configurations::instance()->get("pluginsPath") + "/" + this->id).isDir())
    {
        Log::warning("The plugin doesn't exists", Properties("id", this->id), "ApiConfiguration", "ApiConfiguration");
        return ;
    }
    // Creates the plugin configurations parent node if it doesn't exists
    QDomElement element = this->configuration.writeDom();
    if (element.firstChildElement("configurations").isNull())
        element.appendChild(element.toDocument().createElement("configurations"));
    element = element.firstChildElement("configurations").firstChildElement("plugin");
    this->isInitialized();
    this->configuration.release();
}

ApiConfiguration::~ApiConfiguration()
{
}

QString     ApiConfiguration::getPath() const
{
    return (this->id);
}

QString     ApiConfiguration::get(const QString &nodeName) const
{
    QString result;

    result = this->_get(nodeName, this->_findConfiguration(this->configuration.readDom()));
    this->configuration.release();
    return (result);
}

unsigned int     ApiConfiguration::count(const QString &nodeName) const
{
    unsigned int result;

    result = this->_count(nodeName, this->_findConfiguration(this->configuration.readDom()));
    this->configuration.release();
    return (result);
}

LightBird::IConfiguration &ApiConfiguration::set(const QString &nodeName, const QString &nodeValue)
{
    this->_set(nodeName, nodeValue, this->_findConfiguration(this->configuration.writeDom()));
    this->configuration.release();
    return (*this);
}

bool        ApiConfiguration::remove(const QString &nodeName)
{
    bool    result;

    result = this->_remove(nodeName, this->_findConfiguration(this->configuration.writeDom()));
    this->configuration.release();
    return (result);
}

QDomElement ApiConfiguration::readDom() const
{
    return (this->_findConfiguration(this->configuration.readDom()));
}

QDomElement ApiConfiguration::writeDom()
{
    return (this->_findConfiguration(this->configuration.writeDom()));
}

void        ApiConfiguration::release() const
{
    this->configuration.release();
}

bool        ApiConfiguration::save()
{
    return (this->configuration.save());
}

QDomElement ApiConfiguration::_findConfiguration(QDomElement dom) const
{
    dom = dom.firstChildElement("configurations");
    for (dom = dom.firstChildElement("plugin"); !dom.isNull() && dom.attribute("id") != this->id; dom = dom.nextSiblingElement("plugin"))
        ;
    return (dom);
}
