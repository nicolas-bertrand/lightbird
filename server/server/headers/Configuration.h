#ifndef CONFIGURATION_H
# define CONFIGURATION_H

# include <QFile>
# include <QObject>
# include <QDomDocument>
# include <QReadWriteLock>

# include "IConfiguration.h"

/// @brief Server implementation of the API's IConfiguration.
/// Manage an XML configuration file. This class is thread safe.
class Configuration : public QObject,
                      public LightBird::IConfiguration
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IConfiguration)

public:
    /// @param configurationPath : The path to the configuration file. If the file doesn't exists,
    /// it will be created using the file pointed by alternativePath (usualy a resource).
    /// @param alternativePath : The content of this file will be used to create the configuration
    /// if it doesn't exists. If this parameter is empty and configurationPath doesn't exists,
    /// Configuration will not try to creates the file.
    Configuration(const QString &configurationPath, const QString &alternativePath = "", QObject *parent = 0);
    ~Configuration();
    Configuration(const Configuration &);
    Configuration &operator=(const Configuration &);

    /// @brief If the configuration is loaded.
    operator            bool();
    /// @see LightBird::IConfiguration::getPath
    virtual QString     getPath();
    /// @see LightBird::IConfiguration::get
    virtual QString     get(const QString &nodeName);
    /// @see LightBird::IConfiguration::count
    virtual unsigned    count(const QString &nodeName);
    /// @see LightBird::IConfiguration::set
    virtual void        set(const QString &nodeName, const QString &nodeValue);
    /// @see LightBird::IConfiguration::remove
    virtual bool        remove(const QString &nodeName);
    /// @see LightBird::IConfiguration::readDom
    virtual QDomElement readDom();
    /// @see LightBird::IConfiguration::writeDom
    virtual QDomElement writeDom();
    /// @see LightBird::IConfiguration::release
    virtual void        release();
    /// @see LightBird::IConfiguration::save
    virtual bool        save();
    /// @brief Set the parent of the object is his living thread (not in the current thread)
    void                setParent(QObject *parent);

signals:
    void                setParentSignal(QObject *parent);

protected:
    Configuration();
    /// @brief Load the xml configuration file.
    /// @param path : The path to the file.
    /// @return If the file has been correctly loaded.
    bool                _load(const QString &configurationPath, const QString &alternativePath);
    /// @param root : The root element of the nodeName.
    virtual QString     _get(const QString &nodeName, QDomElement root);
    /// @param root : The root element of the nodeName.
    virtual unsigned    _count(const QString &nodeName, QDomElement root);
    /// @param root : The root element of the nodeName.
    virtual void        _set(const QString &nodeName, const QString &nodeValue, QDomElement root);
    /// @param root : The root element of the nodeName.
    virtual bool        _remove(const QString &nodeName, QDomElement root);

private slots:
    /// @brief Set the parent of the configuration in his living thread (because this can't be done in an other thread)
    void                _setParent(QObject *parent);

protected:
    QReadWriteLock      mutex;  ///< Make the configuration thread safe.
    QDomDocument        doc;    ///< The in-memory DOM representation of the XML document.
    QDomElement         dom;    ///< The root of the XML document.
    QFile               file;   ///< The XML configuration file.
    bool                loaded; ///< If a configuration is loaded.
};

#endif // CONFIGURATION_H
