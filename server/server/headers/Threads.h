#ifndef THREADS_H
# define THREADS_H

# include <QMap>
# include <QMutex>
# include <QObject>
# include <QThread>

/// @brief Manages the threads of the server. Ensure that the threads are correctly
/// deleted and cleaned, in a safe way.
class Threads : public QObject
{
    Q_OBJECT

public:
    Threads(QObject *parent = 0);
    ~Threads();

    /// @brief Adds a new thread to manage, and start it.
    /// @param thread : The thread to handle.
    /// @param remove : If the object has to be deleted when the thread finished.
    void            newThread(QThread *thread, bool remove = true);
    /// @brief Deletes a managed thread. This method just calls QThread::quit()
    /// in order to quit the event loop of the thread and stop is. Then the thread
    /// is cleaned by the slot _threadFinished with catch the signal finished of the thread.
    /// @param thread : The thread to handle.
    void            deleteThread(QThread *thread);
    /// @brief Removes a thread from the managed list.
    /// The thread will no longer be managed.
    bool            removeThread(QThread *thread);
    /// @brief Stops and cleans all the thread managed by the instance.
    /// This method will returns only when all the threads are stopped.
    void            shutdown();
    /// @brief Returns the instance of this class created by the Server.
    static Threads  *instance();

private slots:
    /// @brief Cleans a thread that has finished.
    void            _threadFinished();
    /// @brief Cleans a thread that has been deleted.
    void            _threadDestroyed(QObject *object);

private:
    Threads(const Threads &);
    Threads &operator=(const Threads &);

    QMap<QThread *, bool> threads; ///< The list of the managed threads. The value tells if the object has to be deleted when the thread is finished.
    QMutex                mutex;   ///< Lock the member threads.
};

#endif // THREADS_H
