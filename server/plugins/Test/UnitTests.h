#ifndef UNITTESTS_H
# define UNITTESTS_H

#include "Plugin.h"

class UnitTests
{
public:
    UnitTests(LightBird::IApi &api);
    ~UnitTests();

private:
    UnitTests(const UnitTests &);
    UnitTests &operator=(const UnitTests &);

    bool    _configuration();
    bool    _accounts();
    bool    _collections();
    bool    _directories();
    bool    _events();
    bool    _files();
    bool    _groups();
    bool    _limits();
    bool    _permissions();
    bool    _tags();

    LightBird::IApi      &api;
    LightBird::IDatabase &database;
    LightBird::ILogs     &log;
};

// Macro used by the unit tests.
# define ASSERT(a) if (!(a))\
{\
    QMap<QString, QString> properties;\
    properties.insert("line", QString::number(__LINE__));\
    throw (properties);\
}

#endif // UNITTESTS_H
