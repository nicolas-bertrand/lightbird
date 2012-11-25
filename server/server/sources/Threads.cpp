#include "Log.h"
#include "Server.h"
#include "Mutex.h"
#include "Threads.h"

Threads::Threads(QObject *parent) : QObject(parent)
{
}

Threads::~Threads()
{
    this->shutdown();
    LOG_TRACE("Threads destroyed!", "Threads", "~Threads");
}

void    Threads::newThread(QThread *thread, bool remove)
{
    Mutex   mutex(this->mutex, "Threads", "newThread");

    if (!mutex)
        return ;
    if (this->threads.contains(thread))
        Log::warning("The thread is already handled", Properties("thread", QString::number((quint64)thread, 16)), "Threads", "newThread");
    else if (thread->isFinished())
        Log::warning("This thread is already finished", Properties("thread", QString::number((quint64)thread, 16)), "Threads", "newThread");
    else
    {
        LOG_TRACE("New thread", Properties("thread", QString::number((quint64)thread, 16)), "Threads", "newThread");
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
}

void    Threads::deleteThread(QThread *thread)
{
    Mutex   mutex(this->mutex, "Threads", "deleteThread");

    if (!mutex)
        return ;
    if (this->threads.contains(thread) == true)
    {
        LOG_TRACE("Deleting the thread", Properties("thread", QString::number((quint64)thread, 16)), "Threads", "deleteThread");
        // Quit the event loop of the thread
        thread->quit();
    }
}

void    Threads::shutdown()
{
    Mutex   mutex(this->mutex, "Threads", "shutdown");

    if (!mutex)
        return ;
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
        // Detele the object if it is orphan
        if (it.value() && it.key()->parent() == NULL)
        {
            if (QThread::currentThread() == it.key()->thread())
                delete it.key();
            else
                it.key()->deleteLater();
        }
    }
    this->threads.clear();
}

void    Threads::_threadFinished()
{
    Mutex   mutex(this->mutex, "Threads", "_threadFinished");

    if (!mutex)
        return ;
    QMutableMapIterator<QThread *, bool> it(this->threads);
    while (it.hasNext())
    {
        it.next();
        if (it.key()->isFinished())
        {
            LOG_TRACE("Thread finished", Properties("thread", QString::number((quint64)it.key(), 16)).add("name", it.key()->objectName(), false), "Threads", "_threadFinished");
            // Detele the object if it is orphan
            if (it.value() && it.key()->parent() == NULL)
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
}

void    Threads::_threadDestroyed(QObject *object)
{
    Mutex   mutex(this->mutex, "Threads", "_threadDestroyed");

    if (!mutex)
        return ;
    QMutableMapIterator<QThread *, bool> it(this->threads);
    while (it.hasNext())
    {
        it.next();
        if (it.key() == object)
        {
            // Remove the destroyed thread of the list
            it.remove();
            LOG_TRACE("Thread destroyed", Properties("thread", QString::number((quint64)object, 16)), "Threads", "_threadDestroyed");
            break;
        }
    }
}

Threads *Threads::instance()
{
    return (Server::instance().getThreads());
}
