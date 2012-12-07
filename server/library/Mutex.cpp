#include "Library.h"
#include "LightBird.h"
#include "Mutex.h"

Mutex::Mutex(QMutex &m, const QString &o, const QString &f, int wait)
    : mutex(&m)
    , readWriteLock(NULL)
    , isLock(false)
    , object(o)
    , function(f)
{
    this->lock(wait);
}

Mutex::Mutex(QMutex &m, const QString &p, const QString &o, const QString &f, int wait)
    : mutex(&m)
    , readWriteLock(NULL)
    , isLock(false)
    , plugin(p)
    , object(o)
    , function(f)
{
    this->lock(wait);
}

Mutex::Mutex(QReadWriteLock &m, const QString &o, const QString &f, int wait)
    : mutex(NULL)
    , readWriteLock(&m)
    , isLock(false)
    , object(o)
    , function(f)
{
    this->lockForWrite(wait);
}

Mutex::Mutex(QReadWriteLock &m, const QString &p, const QString &o, const QString &f, int wait)
    : mutex(NULL)
    , readWriteLock(&m)
    , isLock(false)
    , plugin(p)
    , object(o)
    , function(f)
{
    this->lockForWrite(wait);
}

Mutex::Mutex(QReadWriteLock &m, Mutex::ReadWriteLock lock, const QString &o, const QString &f, int wait)
    : mutex(NULL)
    , readWriteLock(&m)
    , isLock(false)
    , object(o)
    , function(f)
{
    if (lock == Mutex::WRITE)
        this->lockForWrite(wait);
    else
        this->lockForRead(wait);
}

Mutex::Mutex(QReadWriteLock &m, Mutex::ReadWriteLock lock, const QString &p, const QString &o, const QString &f, int wait)
    : mutex(NULL)
    , readWriteLock(&m)
    , isLock(false)
    , plugin(p)
    , object(o)
    , function(f)
{
    if (lock == Mutex::WRITE)
        this->lockForWrite(wait);
    else
        this->lockForRead(wait);
}

Mutex::~Mutex()
{
    this->unlock();
}

Mutex::operator bool() const
{
    return (this->isLock);
}

bool    Mutex::lock(int wait)
{
    if (this->isLock)
        return (false);
    if (this->mutex)
    {
        if (!(this->isLock = this->mutex->tryLock(wait)))
            LightBird::Library::log().error("Deadlock", Properties("plugin", this->plugin, false).toMap(), this->object, this->function);
    }
    else
        this->lockForWrite(wait);
    return (this->isLock);
}

bool    Mutex::lockForWrite(int wait)
{
    if (this->isLock)
        return (false);
    if (this->readWriteLock)
    {
        if (!(this->isLock = this->readWriteLock->tryLockForWrite(wait)))
            LightBird::Library::log().error("Deadlock", Properties("plugin", this->plugin, false).toMap(), this->object, this->function);
    }
    else
        this->lock(wait);
    return (this->isLock);
}

bool    Mutex::lockForRead(int wait)
{
    if (this->isLock)
        return (false);
    if (this->readWriteLock)
    {
        if (!(this->isLock = this->readWriteLock->tryLockForRead(wait)))
            LightBird::Library::log().error("Deadlock", Properties("plugin", this->plugin, false).toMap(), this->object, this->function);
    }
    else
        this->lock(wait);
    return (this->isLock);
}

void    Mutex::unlock()
{
    if (this->isLock)
    {
        if (this->mutex)
            this->mutex->unlock();
        else
            this->readWriteLock->unlock();
        this->isLock = false;
    }
}
