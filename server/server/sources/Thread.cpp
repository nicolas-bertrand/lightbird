#include "Thread.h"
#include "ThreadPool.h"
#include "Log.h"

#include <QCoreApplication>

Thread::Thread()
{
    this->moveToThread(this);
    QObject::connect(this, SIGNAL(taskAvailableSignal(ThreadPool::ITask*)), this, SLOT(_taskAvailable(ThreadPool::ITask*)), Qt::QueuedConnection);
}

Thread::~Thread()
{
    Log::trace("Thread destroyed!", "Thread", "~Thread");
}

void    Thread::run()
{
    this->exec();
    this->moveToThread(QCoreApplication::instance()->thread());
    this->deleteLater();
}

void    Thread::taskAvailable(ThreadPool::ITask *task)
{
    emit taskAvailableSignal(task);
}

void    Thread::_taskAvailable(ThreadPool::ITask *task)
{
    task->run();
    ThreadPool::instance()->threadAvailable(this);
}
