#include "Configurations.h"
#include "Log.h"
#include "Server.h"
#include "SmartMutex.h"
#include "Thread.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool(QObject *parent) : QObject(parent)
{
    this->threadsNumber = Configurations::instance()->get("threadsNumber").toUInt();
    for (unsigned int i = 0; i < this->threadsNumber; ++i)
        this->_createThread();
}

ThreadPool::~ThreadPool()
{
    this->shutdown();
    Log::trace("ThreadPool destroyed!", "ThreadPool", "~ThreadPool");
}

void            ThreadPool::addTask(ThreadPool::ITask *task)
{
    SmartMutex  mutex(this->mutex);
    this->tasks.enqueue(task);
    this->_executeTask();
}

unsigned int    ThreadPool::getThreadNumber() const
{
    return (this->threadsNumber);
}

void            ThreadPool::setThreadNumber(unsigned int threadNumber)
{
    SmartMutex  mutex(this->mutex);
    if (threadNumber == 0)
        threadNumber = 1;
    this->threadsNumber = threadNumber;
    for (unsigned int i = this->threads.count(); i < this->threadsNumber; ++i)
        this->_createThread();
    for (unsigned int i = this->threads.count(); i > this->threadsNumber && !this->available.isEmpty(); --i)
        this->_removeThread(this->available.head());
}

void            ThreadPool::shutdown()
{
    SmartMutex  mutex(this->mutex);
    QListIterator<Thread *> it(this->threads);
    // Quits the event loop of each threads
    while (it.hasNext())
        it.next()->quit();
    this->threadsNumber = 0;
    this->threads.clear();
    this->available.clear();
    this->tasks.clear();
    it.toFront();
    mutex.unlock();
    // Waits until each thread is finished
    while (it.hasNext())
        it.next()->wait();
    it.toFront();
    // Destroys the threads
    while (it.hasNext())
        delete it.next();
}

void            ThreadPool::_threadAvailable(Thread *thread)
{
    SmartMutex  mutex(this->mutex);
    if (this->threads.contains(thread) && !this->available.contains(thread))
        this->available.enqueue(thread);
    if (this->threads.count() > (int)this->threadsNumber && !this->available.isEmpty())
        this->_removeThread(this->available.head());
    this->_executeTask();
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

QThread         *ThreadPool::ITask::getThread() const
{
    SmartMutex  mutex(this->mutex);
    return (this->thread);
}

void            ThreadPool::ITask::setThread(QThread *thread)
{
    SmartMutex  mutex(this->mutex);
    this->thread = thread;
}

ThreadPool  *ThreadPool::instance()
{
    return (Server::instance().getThreadPool());
}
