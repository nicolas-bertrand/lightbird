#ifndef THREADPOOL_H
# define THREADPOOL_H

# include <QObject>
# include <QMutex>
# include <QQueue>
# include <QList>

class Thread;

/// @brief Manages a pool of threads used to execute any task.
class ThreadPool : public QObject
{
    Q_OBJECT

public:
    class ITask;
    friend class Thread;

    ThreadPool(QObject *parent = 0);
    ~ThreadPool();

    /// @brief Adds a new task to the queue. It will be executed as soon as a thread
    /// is available. It is the responsability of the caller to free the task.
    void                addTask(ThreadPool::ITask *task);
    /// @brief Returns the number of threads currently in the pool.
    unsigned int        getThreadNumber() const;
    /// @brief Changes the number of threads in the pool.
    void                setThreadNumber(unsigned int threadNumber);
    /// @brief Quits all the threads and waits for them to be finished.
    void                shutdown();
    /// @brief Returns the instance of this class created by the Server.
    static ThreadPool   *instance();

private:
    ThreadPool(const ThreadPool &);
    ThreadPool &operator=(const ThreadPool &);

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
    unsigned int        threadsNumber; ///< The number of threads in the thread pool.
    QMutex              mutex;         ///< Makes this class thread safe.

public:
    /// @brief This interface is used by the thread pool to execute a task.
    /// The run method is called when the task can be executed in a dedicated thread.
    class ITask
    {
    public:
        /// @brief Runs the task.
        virtual void    run() = 0;
        /// @brief Returns the thread in which the task is running, or NULL
        /// if it is not running.
        QThread         *getThread() const;
        /// @brief Allows to set the address of the thread in which the task is running.
        void            setThread(QThread *thread);

    private:
        QThread         *thread; ///< The current thread of the task.
        mutable QMutex  mutex;   ///< Makes this class thread safe.
    };
};

#endif // THREADPOOL_H
