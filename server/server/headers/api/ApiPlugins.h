#ifndef APIPLUGINS_H
# define APIPLUGINS_H

# include <QObject>

# include "IPlugins.h"

/// @brief A class used by the plugins to manipulate plugins.
/// This class is a thread safe singleton.
class ApiPlugins : public QObject,
                   public LightBird::IPlugins
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugins)

public:
    static ApiPlugins   *instance(QObject *parent = 0);

    QSharedPointer<LightBird::IFuture<bool> > load(const QString &id);
    QSharedPointer<LightBird::IFuture<bool> > unload(const QString &id);
    QSharedPointer<LightBird::IFuture<bool> > install(const QString &id);
    QSharedPointer<LightBird::IFuture<bool> > uninstall(const QString &id);
    LightBird::IMetadata getMetadata(const QString &id) const;
    LightBird::IPlugins::State getState(const QString &id);
    QStringList         getPlugins();
    QStringList         getLoadedPlugins();
    QStringList         getUnloadedPlugins();
    QStringList         getInstalledPlugins();
    QStringList         getUninstalledPlugins();

private:
    ApiPlugins(QObject *parent = 0);
    ~ApiPlugins();
    ApiPlugins(const ApiPlugins &);
    ApiPlugins &operator=(const ApiPlugins &);

    static ApiPlugins   *_instance; ///< The instance of the singleton.
};

#endif // APIPLUGINS_H
