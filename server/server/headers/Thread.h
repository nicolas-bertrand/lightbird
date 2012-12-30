#ifndef THREAD_H
# define THREAD_H

# include <QThread>

# include "ThreadPool.h"

/// @brief Represents a thread that executes tasks given by the ThreadPool.
class Thread : public QThread
{
    Q_OBJECT

public:
    Thread();
    ~Thread();

    void    run();
    /// @brief Emits the taskAvailableSignal which calls _taskAvailable in the thread.
    void    taskAvailable(ThreadPool::ITask *task);

signals:
    /// @brief This signal is emitted when a new task is available
    /// for the current thread.
    void    taskAvailableSignal(ThreadPool::ITask *task);

private slots:
    /// @brief Runs a task.
    void    _taskAvailable(ThreadPool::ITask *task);

private:
    Thread(const Thread &);
    Thread &operator=(const Thread &);
};

#endif // THREAD_H
