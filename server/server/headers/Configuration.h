#ifndef CONFIGURATION_H
# define CONFIGURATION_H

# include <QFile>
# include <QObject>
# include <QDomDocument>
# include <QReadWriteLock>

# include "IConfiguration.h"

# include "Initialize.h"

/// @brief Server implementation of the API's IConfiguration.
/// Manage an XML configuration file.
class Configuration : public QObject,
                      public LightBird::Initialize,
                      public LightBird::IConfiguration
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IConfiguration)

public:
    /// @param configurationPath : The path to the configuration file. If the file does not exist,
    /// it will be created using the file pointed by alternativePath (usualy a resource).
    /// @param alternativePath : The content of this file will be used to create the configuration
    /// if it does not exist. If this parameter is empty and configurationPath does not exist,
    /// Configuration will not try to creates the file.
    /// @param server : True if this is the configuration of the server.
    Configuration(const QString &configurationPath, const QString &alternativePath = "", QObject *parent = 0, bool server = false);
    ~Configuration();

    /// @see LightBird::IConfiguration::getPath
    virtual QString     getPath() const;
    /// @see LightBird::IConfiguration::get
    virtual QString     get(const QString &nodeName, const QString &defaultValue = "") const;
    /// @see LightBird::IConfiguration::count
    virtual unsigned    count(const QString &nodeName) const;
    /// @see LightBird::IConfiguration::set
    virtual LightBird::IConfiguration &set(const QString &nodeName, const QString &nodeValue);
    /// @see LightBird::IConfiguration::remove
    virtual bool        remove(const QString &nodeName);
    /// @see LightBird::IConfiguration::readDom
    virtual QDomElement readDom() const;
    /// @see LightBird::IConfiguration::writeDom
    virtual QDomElement writeDom();
    /// @see LightBird::IConfiguration::release
    virtual void        release() const;
    /// @see LightBird::IConfiguration::save
    virtual bool        save();
    /// @brief Set the parent of the object is his living thread (not in the current thread)
    void                setParent(QObject *parent);

signals:
    void                setParentSignal(QObject *parent);

private slots:
    /// @brief Set the parent of the configuration in his living thread (because this can't be done in an other thread)
    void                _setParent(QObject *parent);

protected:
    Configuration();
    Configuration(const Configuration &);
    Configuration &operator=(const Configuration &);

    /// @brief Load the xml configuration file.
    /// @param path : The path to the file.
    /// @return If the file has been correctly loaded.
    bool                _load(const QString &configurationPath, const QString &alternativePath);
    /// @param root : The root element of the nodeName.
    virtual QString     _get(const QString &nodeName, const QString &defaultValue, QDomElement root) const;
    /// @param root : The root element of the nodeName.
    virtual unsigned    _count(const QString &nodeName, QDomElement root) const;
    /// @param root : The root element of the nodeName.
    virtual void        _set(const QString &nodeName, const QString &nodeValue, QDomElement root);
    /// @param root : The root element of the nodeName.
    virtual bool        _remove(const QString &nodeName, QDomElement root);

    bool                    server; ///< True if this instance stores the configuration of the server.
    QDomDocument            doc;    ///< The in-memory DOM representation of the XML document.
    QDomElement             dom;    ///< The root of the XML document.
    QFile                   file;   ///< The XML configuration file.
    mutable QReadWriteLock  mutex;  ///< Make the configuration thread safe.
};

#endif // CONFIGURATION_H
