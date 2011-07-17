#ifndef EXTENSIONS_H
# define EXTENSIONS_H

# include <QMap>
# include <QMutex>
# include <QObject>

# include "IExtension.h"
# include "IExtensions.h"

class Plugin;

/// @brief Manage the extensions of the API, and allows plugins to use them.
class Extensions : public QObject,
                   public LightBird::IExtensions
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IExtensions)

public:
    static Extensions   *instance(QObject *parent = 0);

    /// @brief Adds an extension plugin to manage.
    void                add(Plugin *plugin);
    /// @brief Removes a plugin. It will be really removed when all its extensions will be released.
    void                remove(Plugin *plugin);
    /// @see LightBird::IExtensions::get
    QList<void *>       get(const QString &name);
    /// @see LightBird::IExtensions::release
    void                release(QList<void *> extensions);

signals:
    void                releaseSignal(QString id);

private:
    Extensions(QObject *parent = 0);
    ~Extensions();
    Extensions(const Extensions &);
    Extensions &operator=(const Extensions &);

    void                      _remove(const QString &plugin);

    struct PluginInfo
    {
        Plugin                *instance;   ///< The plugin class.
        LightBird::IExtension *extensions; ///< The IExtension interface of the plugin.
        bool                  loaded;      ///< If false, the plugin can't get more extensions and is going to be removed.
    };

    struct Extension
    {
        QString               plugin;       ///< The id of the plugin that creates this extension.
        void                  *instance;    ///< The instance of the extension.
    };

private slots:
    void                      _release(QString id);

private:
    QMap<QString, PluginInfo> plugins;           ///< The list of the plugins that implements one or more extensions. The key is the id of the plugin.
    QMap<QString, Extension>  extensions;        ///< Contains the extensions currently used. The key is the name of the extension.
    QMap<QString, QString>    extensionsPlugins; ///< Associates the extensions names to the plugins that implements them.
    QMutex                    mutex;             ///< Makes the class members thread safe.
    static Extensions         *_instance;
};

#endif // EXTENSIONS_H
