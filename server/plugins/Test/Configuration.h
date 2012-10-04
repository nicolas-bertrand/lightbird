#ifndef CONFIGURATION_H
# define CONFIGURATION_H

# include "ITest.h"

class Configuration : public ITest
{
public:
    Configuration(LightBird::IApi &api);
    ~Configuration();

    unsigned int    run();

private:
    Configuration(const Configuration &);
    Configuration &operator=(const Configuration &);
};

#endif // CONFIGURATION_H
