#include <QCoreApplication>

#include "Log.h"
#include "Thread.h"
#include "ThreadPool.h"

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
    task->thread = this;
    task->run();
    task->thread = NULL;
    ThreadPool::instance()->_threadAvailable(this);
}