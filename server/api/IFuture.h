#ifndef IFUTURE_H
# define IFUTURE_H

# include <limits.h>

namespace LightBird
{
    /// @brief This classe has the same behavior than QFuture, and is uses for the
    /// same purpose, which is to allow a thread to get the result of an asynchronous
    /// operation, and wait until this result became available.
    /// The type of the result is defined by the type of the template.
    template<class T>
    class IFuture
    {
    public:
        virtual ~IFuture() {}

        /// @brief This method block the caller thread until the result became available,
        /// or the time given in parameter elapsed. The result is then set to the reference
        /// parameter (if it is available). The maximum time to wait for the result can be
        /// defined using the parameter time.
        /// @param result : The result of Future, if it became available during the given time.
        /// @param time : The maximum amount of time in milliseconds to wait for the result. After
        /// that, this method will returns false, and the result is set to the default value. By
        /// default, this method will block infinitely until the result became available.
        /// @return True if the result has been set, and false if the time elapsed.
        virtual bool    getResult(T &result, unsigned long time = ULONG_MAX) = 0;
        /// @brief This method overload getResult and it block infinitely until the result
        /// became available.
        /// @return : The value of the Future.
        virtual T        getResult() = 0;
    };
}

#endif // IFUTURE_H
