#ifndef APIGUIS_H
# define APIGUIS_H

# include <QObject>

# include "IGuis.h"

/// @brief The server implementation of the IGuis interface.
/// Make the connection between IGuis and IGui, and guarantees that the
/// methods of IGui are called from the GUI thread.
class ApiGuis : public QObject,
                public LightBird::IGuis
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IGuis)

public:
    static ApiGuis *instance(QObject *parent = 0);

    void    show(const QString &id = "");
    void    hide(const QString &id = "");
    bool    noGui();

signals:
    void    showSignal(const QString &id);
    void    hideSignal(const QString &id);

private:
    ApiGuis(QObject *parent = 0);
    ~ApiGuis();
    ApiGuis(const ApiGuis &);
    ApiGuis &operator=(const ApiGuis &);

private slots:
    void    _show(const QString &id);
    void    _hide(const QString &id);

private:
    static ApiGuis  *_instance; ///< The instance of the singleton.
    bool            isNoGui;    ///< True if the server is in noGui mode.
};

#endif // APIGUIS_H
