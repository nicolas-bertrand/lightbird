#ifndef APIPLUGINS_H
# define APIPLUGINS_H

# include <QObject>

# include "IPlugins.h"

/// @brief A class class used by the server to manipulate plugins.
/// This class is a thread-safe singleton.
class ApiPlugins : public QObject,
                   public LightBird::IPlugins
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugins)

public:
    static ApiPlugins *instance(QObject *parent = 0);

    /// @see LightBird::IPlugins::load
    QSharedPointer<LightBird::IFuture<bool> > load(const QString &id);
    /// @see LightBird::IPlugins::unload
    QSharedPointer<LightBird::IFuture<bool> > unload(const QString &id);
    /// @see LightBird::IPlugins::install
    QSharedPointer<LightBird::IFuture<bool> > install(const QString &id);
    /// @see LightBird::IPlugins::uninstall
    QSharedPointer<LightBird::IFuture<bool> > uninstall(const QString &id);
    /// @see LightBird::IPlugins::getMetadata
    LightBird::IMetadata getMetadata(const QString &id) const;
    /// @see LightBird::IPlugins::getState
    LightBird::IPlugins::State getState(const QString &id);
    /// @see LightBird::IPlugins::getPlugins
    QStringList     getPlugins();
    /// @see LightBird::IPlugins::getLoadedPlugins
    QStringList     getLoadedPlugins();
    /// @see LightBird::IPlugins::getInstalledPlugins
    QStringList     getInstalledPlugins();
    /// @see LightBird::IPlugins::getUninstalledPlugins
    QStringList     getUninstalledPlugins();

private:
    ApiPlugins(QObject *parent = 0);
    ~ApiPlugins();
    ApiPlugins(const ApiPlugins &);
    ApiPlugins  *operator=(const ApiPlugins &);

    static ApiPlugins   *_instance;
};

#endif // APIPLUGINS_H
