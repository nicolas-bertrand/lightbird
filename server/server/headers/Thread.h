#ifndef THREAD_H
# define THREAD_H

# include <QThread>
# include <QWaitCondition>
# include <QTimer>

# include "ThreadPool.h"

/// @brief Manage the threads of the server. Ensure that thread are correctly
/// deleted and cleaned, in a safe way.
class Thread : public QThread
{
    Q_OBJECT

public:
    Thread();
    ~Thread();

    void    run();
    /// @brief Emit the taskAvailableSignal which calls _taskAvailable in the right thread.
    void    taskAvailable(ThreadPool::ITask *task);

private:
    Thread(const Thread &);
    Thread &operator=(const Thread &);

private slots:
    /// @brief The thread has to run the first task in que thread pool queue.
    void    _taskAvailable(ThreadPool::ITask *task);

signals:
    /// @brief This signal is emited when a new task is available
    /// for the current thread.
    void    taskAvailableSignal(ThreadPool::ITask *task);
};

#endif // THREAD_H
