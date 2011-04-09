#ifndef APIPLUGINS_H
# define APIPLUGINS_H

# include <QObject>

# include "IPlugins.h"

/// @brief A class class used by the server to manipulate plugins.
/// This class is a thread-safe singleton.
class ApiPlugins : public QObject,
                   public Streamit::IPlugins
{
    Q_OBJECT
    Q_INTERFACES(Streamit::IPlugins)

public:
    static ApiPlugins *instance(QObject *parent = 0);

    /// @see Streamit::IPlugins::load
    QSharedPointer<Streamit::IFuture<bool> > load(const QString &id);
    /// @see Streamit::IPlugins::unload
    QSharedPointer<Streamit::IFuture<bool> > unload(const QString &id);
    /// @see Streamit::IPlugins::install
    QSharedPointer<Streamit::IFuture<bool> > install(const QString &id);
    /// @see Streamit::IPlugins::uninstall
    QSharedPointer<Streamit::IFuture<bool> > uninstall(const QString &id);
    /// @see Streamit::IPlugins::getMetadata
    Streamit::IMetadata getMetadata(const QString &id) const;
    /// @see Streamit::IPlugins::getState
    Streamit::IPlugins::State getState(const QString &id);
    /// @see Streamit::IPlugins::getPlugins
    QStringList     getPlugins();
    /// @see Streamit::IPlugins::getLoadedPlugins
    QStringList     getLoadedPlugins();
    /// @see Streamit::IPlugins::getInstalledPlugins
    QStringList     getInstalledPlugins();
    /// @see Streamit::IPlugins::getUninstalledPlugins
    QStringList     getUninstalledPlugins();

private:
    ApiPlugins(QObject *parent = 0);
    ~ApiPlugins();
    ApiPlugins(const ApiPlugins &);
    ApiPlugins  *operator=(const ApiPlugins &);

    static ApiPlugins   *_instance;
};

#endif // APIPLUGINS_H
