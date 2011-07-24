#include "Log.h"
#include "SmartMutex.h"

SmartMutex::SmartMutex(QMutex &m, const QString &o, const QString &f, int wait) : mutex(&m),
                                                                                  readWriteLock(NULL),
                                                                                  isLock(false),
                                                                                  object(o),
                                                                                  function(f)
{
    this->lock(wait);
}

SmartMutex::SmartMutex(QReadWriteLock &m, const QString &o, const QString &f, int wait) : mutex(NULL),
                                                                                          readWriteLock(&m),
                                                                                          isLock(false),
                                                                                          object(o),
                                                                                          function(f)
{
    this->lockForWrite(wait);
}

SmartMutex::SmartMutex(QReadWriteLock &m, SmartMutex::ReadWriteLock lock,
                       const QString &o, const QString &f, int wait) : mutex(NULL),
                                                                       readWriteLock(&m),
                                                                       isLock(false),
                                                                       object(o),
                                                                       function(f)
{
    if (lock == SmartMutex::WRITE)
        this->lockForWrite(wait);
    else
        this->lockForRead(wait);
}

SmartMutex::~SmartMutex()
{
    this->unlock();
}

SmartMutex::operator bool() const
{
    return (this->isLock);
}

bool    SmartMutex::lock(int wait)
{
    if (this->isLock)
        return (false);
    if (this->mutex)
    {
        if (!(this->isLock = this->mutex->tryLock(wait)))
            Log::error("Deadlock", this->object, this->function);
    }
    else
        this->lockForWrite(wait);
    return (this->isLock);
}

bool    SmartMutex::lockForWrite(int wait)
{
    if (this->isLock)
        return (false);
    if (this->readWriteLock)
    {
        if (!(this->isLock = this->readWriteLock->tryLockForWrite(wait)))
            Log::error("Deadlock", this->object, this->function);
    }
    else
        this->lock(wait);
    return (this->isLock);
}

bool    SmartMutex::lockForRead(int wait)
{
    if (this->isLock)
        return (false);
    if (this->readWriteLock)
    {
        if (!(this->isLock = this->readWriteLock->tryLockForRead(wait)))
            Log::error("Deadlock", this->object, this->function);
    }
    else
        this->lock(wait);
    return (this->isLock);
}

void    SmartMutex::unlock()
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
