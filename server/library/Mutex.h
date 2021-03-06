#ifndef LIGHTBIRD_MUTEX_H
# define LIGHTBIRD_MUTEX_H

# include <QMutex>
# include <QReadWriteLock>
# include <QString>

# include "Defines.h"
# include "Export.h"

/// @brief The equivalent of a smart pointer for the mutex.
/// Ensures that the mutex is unlock when the object goes out of scope.
/// Avoids dead locks by using tryLock instead of lock, with a timeout.
class LIB Mutex
{
public:
    /// @brief List the possible types of lock.
    enum ReadWriteLock
    {
        READ,
        WRITE
    };

    /// @brief Tries to lock the mutex. The parameters object and function are
    /// used to log a message if the lock fail after the time out.
    Mutex(QMutex &mutex, const QString &object = "", const QString &function = "", int wait = MAXTRYLOCK);
    Mutex(QMutex &mutex, const QString &plugin, const QString &object, const QString &function, int wait = MAXTRYLOCK);
    /// @brief Tries to lock the mutex for write. The parameters object and
    /// function are used to log a message if the lock fail after the time out.
    Mutex(QReadWriteLock &readWriteLock, const QString &object = "", const QString &function = "", int wait = MAXTRYLOCK);
    Mutex(QReadWriteLock &readWriteLock, const QString &plugin, const QString &object, const QString &function, int wait = MAXTRYLOCK);
    /// @brief Tries to lock the mutex. The parameters object and function are
    /// used to log a message if the lock fail after the time out.
    /// @param lock : If the mutex have to be lock for read or for write.
    Mutex(QReadWriteLock &readWriteLock, Mutex::ReadWriteLock lock, const QString &object = "", const QString &function = "", int wait = MAXTRYLOCK);
    Mutex(QReadWriteLock &readWriteLock, Mutex::ReadWriteLock lock, const QString &plugin, const QString &object, const QString &function, int wait = MAXTRYLOCK);
    /// @brief Unlock the mutex if it is locked.
    ~Mutex();

    /// @brief Returns true if the mutex has been locked via this instance.
    /// It does not checks if the mutex is actually locked.
    operator bool() const;
    /// @brief Lock an unlocked QMutex.
    /// @return False if the lock fail after wait milliseconds.
    bool    lock(int wait = MAXTRYLOCK);
    /// @brief Lock for write an unlocked QReadWriteLock.
    /// @return False if the lock fail after wait milliseconds.
    bool    lockForWrite(int wait = MAXTRYLOCK);
    /// @brief Lock for read an unlocked QReadWriteLock.
    /// @return False if the lock fail after wait milliseconds.
    bool    lockForRead(int wait = MAXTRYLOCK);
    /// @brief Unlock the QMutex or the QReadWriteLock held by the Mutex.
    void    unlock();

private:
    Mutex();
    Mutex(const Mutex &);
    Mutex &operator=(const Mutex &);

    QMutex         *mutex;
    QReadWriteLock *readWriteLock;
    bool           isLock;
    QString        plugin;
    QString        object;
    QString        function;
};

#endif // LIGHTBIRD_MUTEX_H
