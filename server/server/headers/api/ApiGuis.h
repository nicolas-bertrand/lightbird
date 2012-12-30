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
    ApiGuis(QObject *parent = NULL);
    ~ApiGuis();

    void    show(const QString &id = "");
    void    hide(const QString &id = "");
    bool    noGui() const;
    static ApiGuis *instance();

signals:
    void    showSignal(const QString &id);
    void    hideSignal(const QString &id);

private slots:
    void    _show(const QString &id);
    void    _hide(const QString &id);

private:
    ApiGuis(const ApiGuis &);
    ApiGuis &operator=(const ApiGuis &);

    bool    isNoGui; ///< True if the server is in noGui mode.
};

#endif // APIGUIS_H
