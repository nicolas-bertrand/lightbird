#ifndef CONFIGURATION_H
# define CONFIGURATION_H

# include <QFile>
# include <QMap>
# include <QMutex>
# include <QDomDocument>
# include <QDomElement>
# include <QReadWriteLock>

# include "IConfiguration.h"

/**
 * @brief Server implementation of the API's IConfiguration.
 * Manage an XML configuration file. This class is thread safe.
 */
class Configuration : public QObject,
                      public Streamit::IConfiguration
{
    Q_OBJECT
    Q_INTERFACES(Streamit::IConfiguration)

public:
    /**
     * @param configurationPath : The path to the configuration file. If the file doesn't exists,
     * it will be created using the file pointed by alternativePath (usualy a resource).
     * @param alternativePath : The content of this file will be used to create the configuration
     * if it doesn't exists. If this parameter is empty and configurationPath doesn't exists,
     * Configuration will not try to creates the file.
     */
    Configuration(const QString &configurationPath, const QString &alternativePath = "", QObject *parent = 0);
    Configuration();
    ~Configuration();
    Configuration(const Configuration &);
    Configuration &operator=(const Configuration &);

    /// @return If the configuration is loaded.
    bool                    isLoaded();
    /// @see Streamit::IConfiguration::getPath
    QString                 getPath();
    /// @see Streamit::IConfiguration::get
    QString                 get(const QString &nodeName);
    /// @see Streamit::IConfiguration::count
    int                     count(const QString &nodeName);
    /// @see Streamit::IConfiguration::set
    void                    set(const QString &nodeName, const QString &nodeValue);
    /// @see Streamit::IConfiguration::remove
    bool                    remove(const QString &nodeName);
    /// @see Streamit::IConfiguration::readDom
    const QDomElement       &readDom();
    /// @see Streamit::IConfiguration::writeDom
    QDomElement             &writeDom();
    /// @see Streamit::IConfiguration::release
    void                    release();
    /// @see Streamit::IConfiguration::save
    bool                    save();
    /// @brief Set the parent of the object is his living thread (not in the current thread)
    void                    setParent(QObject *parent);
    /// @brief Allows to test the Configuration features get() set() and count().
    bool                    unitTests();

private:
    /**
     * @brief Load the xml configuration file.
     * @param path : The path to the file.
     * @return If the file has been correctly loaded.
     */
    bool                    _load(const QString &configurationPath, const QString &alternativePath);

    QReadWriteLock          domLock;    ///< Make the configuration thread safe.
    QDomDocument            doc;        ///< The in-memory DOM representation of the XML document.
    QDomElement             dom;        ///< The root of the XML document.
    QFile                   file;       ///< The XML configuration file.
    bool                    loaded;     ///< If a configuration is loaded.

private slots:
    /// @brief Set the parent of the configuration in his living thread (because this can't be done in an other thread)
    void    _setParent(QObject *parent);

signals:
    void    setParentSignal(QObject *parent);
};

#endif // CONFIGURATION_H
