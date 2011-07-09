#ifndef THREADPOOL_H
# define THREADPOOL_H

# include <QObject>
# include <QMutex>
# include <QQueue>
# include <QList>

class Thread;

/// @brief Manages a pool of thread used to execute any task.
class ThreadPool : public QObject
{
    Q_OBJECT

public:
    class ITask;
    friend class Thread;

    static ThreadPool   *instance(QObject *parent = 0);
    /// @brief Adds a new task to the queue. It will be executed as soon as a thread
    /// is available. It is the responsability of the caller to free the task.
    void                addTask(ThreadPool::ITask *task);
    /// @brief Returns the number of threads currently in the pool.
    unsigned            getThreadNumber();
    /// @brief Changes the number of threads in the pool.
    void                setThreadNumber(unsigned threadNumber);

private:
    ThreadPool(QObject *parent = 0);
    ThreadPool(const ThreadPool &);
    ThreadPool &operator=(const ThreadPool &);
    ~ThreadPool();

    /// @brief Allows the threads to tell when they are available.
    void                _threadAvailable(Thread *thread);
    /// @brief Creates a new thread.
    void                _createThread();
    /// @brief Removes an idle thread.
    void                _removeThread(Thread *thread);
    /// @brief Tries to find an available thread to execute a task.
    void                _executeTask();

    QQueue<ThreadPool::ITask *> tasks; ///< The list of the tasks to execute.
    QList<Thread *>     threads;       ///< The list of the threads in the pool.
    QQueue<Thread *>    available;     ///< Stores the treads that are waiting to execute a task.
    QMutex              mutex;         ///< Makes this class thread safe.
    unsigned            threadsNumber; ///< The number of threads in the thread pool.
    static ThreadPool   *_instance;    ///< The instance of the singleton.

public:
    /// @brief This interface is used by the thread pool to execute a task.
    /// The run method is called when the task can be executed in a dedicated thread.
    class ITask
    {
    public:
        /// @brief Run the task.
        virtual void    run() = 0;
        /// @brief Returns the thread in which the task is running, or NULL
        /// if it is not running.
        QThread         *getThread();

    private:
        QThread         *thread; ///< The current thread of the task.
        /// Allows the class Thread to update the current thread of the task.
        friend class    Thread;
    };
};

#endif // THREADPOOL_H
