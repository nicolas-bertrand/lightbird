#include "Configurations.h"
#include "defines.h"
#include "Log.h"
#include "Thread.h"
#include "ThreadPool.h"

ThreadPool *ThreadPool::_instance = NULL;

ThreadPool *ThreadPool::instance(QObject *parent)
{
    if (ThreadPool::_instance == NULL)
        ThreadPool::_instance = new ThreadPool(parent);
    return (ThreadPool::_instance);
}

ThreadPool::ThreadPool(QObject *parent) : QObject(parent)
{
    this->threadsNumber = Configurations::instance()->get("threadsNumber").toUInt();
    for (unsigned i = 0; i < this->threadsNumber; ++i)
        this->_createThread();
}

ThreadPool::~ThreadPool()
{
    Log::trace("ThreadPool destroyed!", "ThreadPool", "~ThreadPool");
    this->mutex.lock();
    QListIterator<Thread *> it(this->threads);
    // Exit the event loop of each threads
    while (it.hasNext())
        it.next()->exit();
    it.toFront();
    // Wait until each thread is finished and deleted
    while (it.hasNext())
    {
        it.peekNext()->wait();
        it.next()->deleteLater();
    }
    this->threads.clear();
    this->available.clear();
    this->tasks.clear();
    this->mutex.unlock();
}

void    ThreadPool::addTask(ThreadPool::ITask *task)
{
    this->mutex.lock();
    this->tasks.enqueue(task);
    this->_executeTask();
    this->mutex.unlock();
}

unsigned    ThreadPool::getThreadNumber()
{
    return (this->threadsNumber);
}

void    ThreadPool::setThreadNumber(unsigned threadNumber)
{
    this->mutex.lock();
    if (threadNumber == 0)
        threadNumber = 1;
    this->threadsNumber = threadNumber;
    for (unsigned i = this->threads.count(); i < this->threadsNumber; ++i)
        this->_createThread();
    for (unsigned i = this->threads.count(); i > this->threadsNumber && !this->available.isEmpty(); --i)
        this->_removeThread(this->available.head());
    this->mutex.unlock();
}

void    ThreadPool::_threadAvailable(Thread *thread)
{
    this->mutex.lock();
    if (this->threads.contains(thread) && !this->available.contains(thread))
        this->available.enqueue(thread);
    if (this->threads.count() > (int)this->threadsNumber && !this->available.isEmpty())
        this->_removeThread(this->available.head());
    this->_executeTask();
    this->mutex.unlock();
}

void    ThreadPool::_createThread()
{
    Thread  *thread = new Thread();

    this->threads.append(thread);
    this->available.enqueue(thread);
    thread->start();
}

void    ThreadPool::_removeThread(Thread *thread)
{
    this->threads.removeAll(thread);
    this->available.removeAll(thread);
    thread->quit();
}

void    ThreadPool::_executeTask()
{
    // If a thread is available and there is a pending task, the thread is awaken
    if (!this->tasks.isEmpty())
        if (!this->available.isEmpty())
            this->available.dequeue()->taskAvailable(this->tasks.dequeue());
}

QThread *ThreadPool::ITask::getThread()
{
    return (this->thread);
}
