#ifndef THREADPOOL_H
# define THREADPOOL_H

# include <QObject>
# include <QMutex>
# include <QQueue>
# include <QList>

class Thread;

/// @brief Manage the threads of the server. Ensure that thread are correctly
/// deleted and cleaned, in a safe way.
class ThreadPool : public QObject
{
    Q_OBJECT

public:
    class ITask;
    static ThreadPool   *instance(QObject *parent = 0);
    /// @brief Add a new task to the queue. It will be executed as soon as a thread
    /// is available. It is the responsability of the caller to free the task.
    void                addTask(ThreadPool::ITask *task);
    unsigned int        getThreadNumber();
    void                setThreadNumber(unsigned int threadNumber);
    /// @brief Allows the threads to tell when they are available.
    void                threadAvailable(Thread *thread);

private:
    ThreadPool(QObject *parent = 0);
    ThreadPool(const ThreadPool &);
    ThreadPool &operator=(const ThreadPool &);
    ~ThreadPool();

    /// @brief Create a new thread.
    void                _createThread();
    /// @brief Remove an idle thread.
    void                _removeThread(Thread *thread);
    /// @brief Try to find an available thread to execute a task.
    void                _executeTask();

    QQueue<ThreadPool::ITask *> tasks;  ///< The list of the tasks to execute.
    QList<Thread *>     threads;        ///< The list of the threads currently running.
    QQueue<Thread *>    available;      ///< Contains the treads that are waiting to execute a task.
    QMutex              mutex;          ///< Make this class thread safe.
    unsigned int        threadNumber;   ///< The number of threads in the thread pool.
    static ThreadPool   *_instance;     ///< The instance of the singleton.

public:
    /// @brief This interface is used by the thread pool to execute a task.
    /// The run method is called when the task can be executed in a dedicated thread.
    class ITask
    {
    public:
        /// @brief Run the task.
        virtual void    run() = 0;
    };
};

#endif // THREADPOOL_H
