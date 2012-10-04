#include "IIdentifier.h"

#include "Plugin.h"
#include "Timer.h"
#include "TableFiles.h"

Timer::Timer(LightBird::IApi &api) : api(api)
{
}

Timer::~Timer()
{
}

bool    Timer::timer(const QString &name)
{
    if (name == TIMER_IDENTIFY)
        return (this->_identify());
    else if (name == TIMER_TIMEOUT)
        return (this->_timeout());
    return (false);
}

void    Timer::identify(const QString &idFile)
{
    this->mutex.lock();
    this->identifyList << idFile;
    this->mutex.unlock();
    this->api.timers().setTimer(TIMER_IDENTIFY);
}

void          Timer::startTimeout(const QString &idClient)
{
    QDateTime newTimeout;

    this->mutex.lock();
    newTimeout = QDateTime::currentDateTime().addSecs(Plugin::getConfiguration().timeout);
    this->timeout.insert(idClient, newTimeout);
    if (this->nextTimeout.isNull())
    {
        this->nextTimeout = newTimeout;
        this->api.timers().setTimer(TIMER_TIMEOUT, this->nextTimeout.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch());
    }
    this->mutex.unlock();
}

void          Timer::stopTimeout(const QString &idClient)
{
    QDateTime oldTimeout;

    this->mutex.lock();
    if (this->timeout.contains(idClient))
    {
        oldTimeout = this->timeout.value(idClient);
        this->timeout.remove(idClient);
        if (this->nextTimeout == oldTimeout)
        {
            this->nextTimeout = QDateTime();
            if (!this->timeout.isEmpty())
            {
                QListIterator<QDateTime> l(this->timeout.values());
                while (l.hasNext())
                    if (l.next() < this->nextTimeout || this->nextTimeout.isNull())
                        this->nextTimeout = l.peekPrevious();
                this->api.timers().setTimer(TIMER_TIMEOUT, this->nextTimeout.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch());
            }
            else
                this->api.timers().removeTimer(TIMER_TIMEOUT);
        }
    }
    this->mutex.unlock();
}

bool    Timer::_identify()
{
    QList<void *>         extensions;
    QStringList           files;
    LightBird::TableFiles file;
    LightBird::IIdentify::Information information;

    // Gets the files to identify
    this->mutex.lock();
    (files = this->identifyList).removeDuplicates();
    this->identifyList.clear();
    this->mutex.unlock();
    // While there are files to identify
    while (!files.isEmpty())
    {
        QStringListIterator it(files);
        while (it.hasNext())
            // Identify the file
            if (file.setId(it.next()))
            {
                if (!(extensions = this->api.extensions().get("IIdentifier")).isEmpty())
                    information = static_cast<LightBird::IIdentifier *>(extensions.first())->identify(file.getFullPath());
                this->api.extensions().release(extensions);
                file.setType(information.type_string);
                file.setInformations(information.data);
                information.data.clear();
            }
        files.clear();
        // If some files have been uploaded in the meantime, we continue the identification
        this->mutex.lock();
        (files = this->identifyList).removeDuplicates();
        this->identifyList.clear();
        this->mutex.unlock();
    }
    return (false);
}

bool          Timer::_timeout()
{
    QDateTime currentTime;
    bool      result;

    this->mutex.unlock();
    this->nextTimeout = QDateTime();
    currentTime = QDateTime::currentDateTime().addSecs(1);
    QMutableHashIterator<QString, QDateTime> it(this->timeout);
    while (it.hasNext())
        // Disconnects the timeout clients
        if (it.next().value() <= currentTime)
        {
            this->api.network().disconnect(it.key());
            it.remove();
        }
        // Searches the next timeout
        else if (it.value() < this->nextTimeout || this->nextTimeout.isNull())
            this->nextTimeout = it.value();
    // Sets the next timeout
    if (this->nextTimeout.isValid())
        this->api.timers().setTimer(TIMER_TIMEOUT, this->nextTimeout.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch());
    result = this->nextTimeout.isValid();
    this->mutex.unlock();
    return (result);
}
