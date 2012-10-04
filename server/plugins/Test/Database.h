#ifndef DATABASE_H
# define DATABASE_H

# include "ITest.h"

class Database : public ITest
{
public:
    Database(LightBird::IApi &api);
    ~Database();

    unsigned int    run();

private:
    Database(const Database &);
    Database &operator=(const Database &);

    void    _accounts();
    void    _collections();
    void    _directories();
    void    _events();
    void    _files();
    void    _groups();
    void    _limits();
    void    _permissions();
    void    _sessions();
    void    _tags();
};

#endif // DATABASE_H
