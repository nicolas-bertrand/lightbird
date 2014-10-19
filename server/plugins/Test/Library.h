#ifndef LIBRARY_H
# define LIBRARY_H

# include <QObject>

# include "ITest.h"

/// @brief Tests the Library of the server.
class Library
    : public QObject
    , public ITest
{
    Q_OBJECT

public:
    Library(LightBird::IApi &api);
    ~Library();

    unsigned int    run();

private:
    Library(const Library &);
    Library &operator=(const Library &);
};

#endif // LIBRARY_H
