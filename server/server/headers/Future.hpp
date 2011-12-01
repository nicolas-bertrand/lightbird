#ifndef FUTURE_H
# define FUTURE_H

# include <QMutex>
# include <QWaitCondition>

# include "IFuture.h"

/// @brief This class is equivalent to QFuture, and is used by the server to the
/// same purpose. The members pointers are shared between all the instances that
/// have been created using the copy constructor. They are deleted when the last
/// instance of the class is deleted (e.g when counter reach zero).
/// When a thread calls getResult(), it is blocked until the result is defined using
/// setResult().
/// @see LightBird::IFuture
template <class T>
class Future : public LightBird::IFuture<T>
{
public:
    /// @see _initialize
    Future()
    {
        this->_initialize(T());
    }
    /// @see _initialize
    Future(T defaultValue)
    {
        this->_initialize(defaultValue);
    }

    /// @brief Decrements counter, and delete the members if it reach zero.
    /// If the original instance (created by the normal constructor) is destroyed
    /// before calling setResult, it will calls wakeAll and set the result to
    /// its default value.
    ~Future()
    {
        this->lockCounter->lock();
        if (this->original && !(*(this->resultSet)))
        {
            this->lockResult->lock();
            *(this->resultSet) = true;
            this->lockResult->unlock();
            this->waitResult->wakeAll();
        }
        if ((--(*(this->counter))) <= 0)
        {
            delete this->result;
            delete this->resultSet;
            delete this->lockResult;
            delete this->waitResult;
            delete this->counter;
            this->lockCounter->unlock();
            delete this->lockCounter;
        }
        else
            this->lockCounter->unlock();
    }

    /// @brief Copy the instance of the Future in parameter, and increments
    /// the counter.
    Future(const Future &future)
    {
        future.lockCounter->lock();
        this->result = future.result;
        this->lockResult = future.lockResult;
        this->lockResult->lock();
        this->resultSet = future.resultSet;
        this->lockResult->unlock();
        this->waitResult = future.waitResult;
        this->counter = future.counter;
        ++(*(this->counter));
        this->lockCounter = future.lockCounter;
        this->original = false;
        future.lockCounter->unlock();
    }

    /// @brief This method block the caller thread until the result is set using setResult
    /// or the original object is destroyed. The value is then set to the referenced parameter.
    /// The maximum time to wait for the result can be defined using the parameter time.
    /// @param result : The result of Future (if no timeout).
    /// @param time : The maximum amount of time in milliseconds to wait for the result. After
    /// that, this method will returns false, and the result is not modified.
    /// @return True if the result has been set, and false if the time elapsed.
    bool        getResult(T &result, unsigned int time = ULONG_MAX)
    {
        bool    wait = true;

        this->lockResult->lock();
        if (this->original)
        {
            result = *(this->result);
            this->lockResult->unlock();
            return (true);
        }
        if (!(*(this->resultSet)))
            wait = this->waitResult->wait(this->lockResult, time);
        result = *(this->result);
        this->lockResult->unlock();
        return (wait);
    }

   /// @brief This method block the caller thread until the result is set using setResult
   /// or the original object is destroyed. The value is then returned.
   /// @return : The value of the Future, when it becomes available.
   T       getResult()
   {
       T   result;

       this->getResult(result);
       return (result);
   }

   /// @brief Set the value of the Future and make it available to the other threads by
   /// calling wakeAll. The result can be set only one time, and only by the original
   /// instance. Otherwise it has no effects.
   /// @param result : The result of the Future.
    void    setResult(T result)
    {
        this->lockResult->lock();
        if (*(this->resultSet) || !this->original)
        {
            this->lockResult->unlock();
            return ;
        }
        *(this->resultSet) = true;
        *(this->result) = result;
        this->lockResult->unlock();
        this->waitResult->wakeAll();
    }

private:
    Future &operator=(const Future &future);

    /// @brief Creates a new future and instanciate all the members.
    /// It also set the counter to one.
    /// The method setResult wakeAll the waiting threads, and must be called
    /// from this instance (nothing appends if it is called from the other
    /// instances created by copy). If this instance is destroyed before
    /// calling setResult, wakeAll is called, and the result value is
    /// the default value.
    /// @param defaultValue : The default value of the result.
    void    _initialize(T defaultValue)
    {
        this->result = new T;
        *(this->result) = defaultValue;
        this->resultSet = new bool;
        *(this->resultSet) = false;
        this->lockResult = new QMutex;
        this->waitResult = new QWaitCondition;
        this->counter = new int;
        *(this->counter) = 1;
        this->lockCounter = new QMutex;
        this->original = true;
    }

    // These variables are created by the constructor, and destroyed by the destructor
    // only when counter is egual to zero.
    T               *result;        ///< Contain the future result. The value is set only one time by setResult (further calls of setResult have no effects).
    bool            *resultSet;     ///< If the result has been set using setResult.
    QWaitCondition  *waitResult;    ///< Allows thread to wait until the result is set using setResult.
    QMutex          *lockResult;    ///< This mutex is used with waitRetult.
    int             *counter;       ///< Number of the instance of the class that pointed to the same result. Used to delete all the allocated pointers when it reach 0.
    QMutex          *lockCounter;   ///< Make the counter thread safe.
    bool            original;       ///< If this instance is the one which created the members and locked the result.
};

#endif // FUTURE_H
