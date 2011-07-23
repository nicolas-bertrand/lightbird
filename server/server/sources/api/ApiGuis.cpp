#include <QApplication>

#include "IGui.h"

#include "ApiGuis.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Server.h"

ApiGuis::ApiGuis(QObject *parent) : QObject(parent)
{
    Log::trace("ApiGuis created", "ApiGuis", "~ApiGuis");
    QObject::connect(this, SIGNAL(showSignal(QString)), this, SLOT(_show(QString)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(hideSignal(QString)), this, SLOT(_hide(QString)), Qt::QueuedConnection);
    this->isNoGui = true;
    if (qobject_cast<QApplication *>(QCoreApplication::instance()))
        this->isNoGui = false;
}

ApiGuis::~ApiGuis()
{
    Log::trace("ApiGuis destroyed!", "ApiGuis", "~ApiGuis");
}

void    ApiGuis::show(const QString &id)
{
    if (!this->isNoGui)
        emit this->showSignal(id);
}

void    ApiGuis::hide(const QString &id)
{
    if (!this->isNoGui)
        emit this->hideSignal(id);
}

bool    ApiGuis::noGui()
{
    return (this->isNoGui);
}

void    ApiGuis::_show(const QString &id)
{
    LightBird::IGui *instance;

    if (id.isEmpty())
    {
        QMapIterator<QString, LightBird::IGui *> it(Plugins::instance()->getInstances<LightBird::IGui>());
        while (it.hasNext())
        {
            it.peekNext().value()->show();
            Plugins::instance()->release(it.next().key());
        }
    }
    else if ((instance = Plugins::instance()->getInstance<LightBird::IGui>(id)))
    {
        instance->show();
        Plugins::instance()->release(id);
    }
}

void    ApiGuis::_hide(const QString &id)
{
    LightBird::IGui *instance;

    if (id.isEmpty())
    {
        QMapIterator<QString, LightBird::IGui *> it(Plugins::instance()->getInstances<LightBird::IGui>());
        while (it.hasNext())
        {
            it.peekNext().value()->hide();
            Plugins::instance()->release(it.next().key());
        }
    }
    else if ((instance = Plugins::instance()->getInstance<LightBird::IGui>(id)))
    {
        instance->hide();
        Plugins::instance()->release(id);
    }
}

ApiGuis *ApiGuis::instance()
{
    return (Server::instance().getApiGuis());
}
