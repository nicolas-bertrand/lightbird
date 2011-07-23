#ifndef APIPLUGINS_H
# define APIPLUGINS_H

# include <QObject>

# include "IPlugins.h"

/// @brief The server implementation of IPlugin which allows plugins to
/// manipulate plugins.
class ApiPlugins : public QObject,
                   public LightBird::IPlugins
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugins)

public:
    ApiPlugins(QObject *parent = NULL);
    ~ApiPlugins();

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
    static ApiPlugins   *instance();

private:
    ApiPlugins(const ApiPlugins &);
    ApiPlugins &operator=(const ApiPlugins &);
};

#endif // APIPLUGINS_H
