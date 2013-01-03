#ifndef NETWORK_H
# define NETWORK_H

# include <QObject>

# include "ITest.h"

/// @brief Tests the Network of the server.
class Network : public QObject,
                public ITest
{
    Q_OBJECT

public:
    Network(LightBird::IApi &api);
    ~Network();

    unsigned int    run();

private:
    Network(const Network &);
    Network &operator=(const Network &);
};

#endif // NETWORK_H
