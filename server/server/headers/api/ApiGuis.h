#ifndef APIGUIS_H
# define APIGUIS_H

# include <QObject>

# include "IGuis.h"

/// @brief The server implementation of the IGuis interface.
/// Make the connection between IGuis and IGui, and guaranteed that the
/// methods of IGui are calles from the GUI thread.
class ApiGuis : public QObject,
                public LightBird::IGuis
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IGuis)

public:
    static ApiGuis *instance(QObject *parent = 0);

    /// @see LightBird::IGuis::show
    void    show(const QString &id = "");
    /// @see LightBird::IGuis::hide
    void    hide(const QString &id = "");
    /// @see LightBird::IGuis::noGui
    bool    noGui();

signals:
    void    showSignal(const QString &id);
    void    hideSignal(const QString &id);

private:
    ApiGuis(QObject *parent = 0);
    ~ApiGuis();
    ApiGuis(const ApiGuis &);
    ApiGuis  *operator=(const ApiGuis &);

    static ApiGuis  *_instance;
    bool            isNoGui;

private slots:
    void    _show(const QString &id);
    void    _hide(const QString &id);
};

#endif // APIGUIS_H
