#ifndef UNITTESTS_H
# define UNITTESTS_H

# include <QMap>
# include <QString>

# include "Plugin.h"

class UnitTests
{
public:
    UnitTests(LightBird::IApi &api);
    ~UnitTests();

private:
    UnitTests(const UnitTests &);
    UnitTests &operator=(const UnitTests &);

    bool    _accounts();
    bool    _collections();
    bool    _configuration();
    bool    _directories();
    bool    _events();
    bool    _files();
    bool    _groups();
    bool    _limits();
    bool    _permissions();
    bool    _sessions();
    bool    _tags();

    LightBird::IApi      &api;
    LightBird::IDatabase &database;
    LightBird::ILogs     &log;
};

#endif // UNITTESTS_H
