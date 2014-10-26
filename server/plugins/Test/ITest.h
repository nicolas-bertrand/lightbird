#ifndef ITEST_H
# define ITEST_H

# include "IApi.h"

/// @brief This interface is implemented by all the tests.
class   ITest
{
public:
    ITest(LightBird::IApi &a) : _api(a), _database(a.database()), _log(a.log()) {}
    virtual ~ITest() { }

    /// @brief Runs the tests.
    /// @return Zero if everything went well, or the line of the failed test.
    virtual unsigned int    run() = 0;

protected:
    ITest(const ITest &);
    ITest &operator=(const ITest &);

    LightBird::IApi      &_api;
    LightBird::IDatabase &_database;
    LightBird::ILogs     &_log;
};

// Throws an exception if the assertion is false
# define ASSERT(a)\
if (!(a))\
    throw ((unsigned int)__LINE__);\
else (void)0

#endif // ITEST_H
