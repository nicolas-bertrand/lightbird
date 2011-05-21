#ifndef THREADS_H
# define THREADS_H

# include <QMap>
# include <QMutex>
# include <QObject>
# include <QThread>

/// @brief Manage the threads of the server. Ensure that thread are correctly
/// deleted and cleaned, in a safe way.
class Threads : public QObject
{
    Q_OBJECT

public:
    static Threads  *instance(QObject *parent = 0);
    /// @brief If the Threads instance is loaded
    static bool     isLoaded();

    /// @brief Add a new thread to manage, and start it.
    /// @param thread : The thread to handle.
    /// @param remove : If the object has to be deleted when the thread finished.
    void            newThread(QThread *thread, bool remove = true);
    /// @brief Delete a managed thread. This method just calls QThread::quit()
    /// in order to quit the event loop of the thread and stop is. Then the thread
    /// is cleaned by the slot _threadFinished with catch the signal finished of the thread.
    /// @param thread : The thread to handle.
    void            deleteThread(QThread *thread);
    /// @brief Stop and clean all the thread managed by this singleton.
    /// This method will returns only when all the threads are stopped.
    void            deleteAll();

private:
    Threads(QObject *parent = 0);
    Threads(const Threads &);
    Threads &operator=(const Threads &);
    ~Threads();

    QMap<QThread *, bool>   threads;        ///< The list of the managed threads. The value tells if the object has to be deleted when the thread is finished.
    QMutex                  lockThreads;    ///< Lock the member threads.
    static Threads          *_instance;     ///< The instance of the singleton

private slots:
    /// @brief Clean a thread that has finished.
    void                _threadFinished();
    /// @brief Clean a thread that has been destroyed.
    void                _threadDestroyed(QObject *object);
};

#endif // THREADS_H
