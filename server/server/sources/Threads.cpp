#include <QTime>
#include <QThreadPool>

#include "Defines.h"
#include "Threads.h"
#include "Log.h"
#include "Configurations.h"

Threads *Threads::_instance = NULL;

Threads::Threads(QObject *parent) : QObject(parent)
{
}

bool    Threads::isLoaded()
{
    if (Threads::_instance)
        return (true);
    return (false);
}

Threads::~Threads()
{
    // Detele all the remaining threads before the destruction of the singleton
    this->deleteAll();
    Log::trace("Threads destroyed!", "Threads", "~Threads");
}

Threads *Threads::instance(QObject *parent)
{
    if (_instance == NULL)
        _instance = new Threads(parent);
    return (_instance);
}

void    Threads::newThread(QThread *thread, bool remove)
{
    if (!this->lockThreads.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Threads", "newThread");
        return ;
    }
    if (this->threads.contains(thread))
        Log::warning("The thread is already handled", Properties("thread", QString::number((quint64)thread, 16)), "Threads", "newThread");
    else if (thread->isFinished())
        Log::warning("This thread is already finished", Properties("thread", QString::number((quint64)thread, 16)), "Threads", "newThread");
    else
    {
        Log::trace("New thread", Properties("thread", QString::number((quint64)thread, 16)), "Threads", "newThread");
        this->threads.insert(thread, remove);
        // When the thread will be finished, we will be notified.
        if (remove)
            QObject::connect(thread, SIGNAL(finished()), this, SLOT(_threadFinished()));
        // If the thread is removed by a third party, we will be notified.
        else
            QObject::connect(thread, SIGNAL(destroyed(QObject*)), this, SLOT(_threadDestroyed(QObject*)));
        // Start the thread if it is not already running
        if (thread->isRunning() == false)
            thread->start();
    }
    this->lockThreads.unlock();
}

void    Threads::deleteThread(QThread *thread)
{
    if (!this->lockThreads.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Threads", "deleteThread");
        return ;
    }
    if (this->threads.contains(thread) == true)
    {
        Log::trace("Deleting the thread", Properties("thread", QString::number((quint64)thread, 16)), "Threads", "deleteThread");
        // Quit the event loop of the thread
        thread->quit();
    }
    this->lockThreads.unlock();
}

void        Threads::deleteAll()
{
    if (!this->lockThreads.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Threads", "deleteAll");
        return ;
    }
    // If all the threads are already deleted
    if (this->threads.isEmpty())
    {
        this->lockThreads.unlock();
        return ;
    }
    Log::info("Deleting all the threads", "Threads", "deleteAll");
    QMutableMapIterator<QThread *, bool> it(this->threads);
    // Quit the event loop of all the threads
    while (it.hasNext())
    {
        it.next();
        QObject::disconnect(it.key(), NULL, this, NULL);
        it.key()->quit();
    }
    it.toFront();
    // Then wait that all the threads finish
    while (it.hasNext())
        it.next().key()->wait();
    it.toFront();
    // Finally, destroy the threads
    while (it.hasNext())
    {
        it.next();
        // Detele the object if it is orphan, and is not the log manager
        if (it.value() && it.key()->parent() == NULL && it.key() != Log::instance())
        {
            if (QThread::currentThread() == it.key()->thread())
                delete it.key();
            else
                it.key()->deleteLater();
        }
    }
    this->threads.clear();
    this->lockThreads.unlock();
}

void        Threads::_threadFinished()
{
    if (!this->lockThreads.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Threads", "_threadFinished");
        return ;
    }
    QMutableMapIterator<QThread *, bool> it(this->threads);
    while (it.hasNext())
    {
        it.next();
        if (it.key()->isFinished())
        {
            Log::trace("Thread finished", Properties("thread", QString::number((quint64)it.key(), 16)).add("name", it.key()->objectName(), false), "Threads", "_threadFinished");
            // Detele the object if it is orphan, and is not the log manager
            if (it.value() && it.key()->parent() == NULL && it.key() != Log::instance())
            {
                if (QThread::currentThread() == it.key()->thread())
                    delete it.key();
                else
                    it.key()->deleteLater();
            }
            // Removes the finished thread of the list
            it.remove();
            break;
        }
    }
    this->lockThreads.unlock();
}

void        Threads::_threadDestroyed(QObject *object)
{
    if (!this->lockThreads.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Threads", "_threadDestroyed");
        return ;
    }
    QMutableMapIterator<QThread *, bool> it(this->threads);
    while (it.hasNext())
    {
        it.next();
        if (it.key() == object)
        {
            // Remove the destroyed thread of the list
            it.remove();
            Log::trace("Thread destroyed", Properties("thread", QString::number((quint64)object, 16)), "Threads", "_threadDestroyed");
            break;
        }
    }
    this->lockThreads.unlock();
}
