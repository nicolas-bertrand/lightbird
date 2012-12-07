#include "Arguments.h"

Arguments::Arguments()
    : argc(0)
    , argv(NULL)
{
}

Arguments::Arguments(int ac, char **av)
    : argc(ac)
    , argv(av)
{
    for (int i = 0; i < argc; ++i)
        this->raw << argv[i];
    QStringListIterator it(this->raw);
    while (it.hasNext())
    {
        it.next();
        // Search the configuration path
        if (it.peekPrevious() == "-c" && it.hasNext())
            this->arguments["configuration"] = it.peekNext();
        // Search the noGui
        else if (it.peekPrevious().toLower() == "-nogui")
            this->arguments["nogui"] = "true";
    }
}

Arguments::~Arguments()
{
}

Arguments::Arguments(const Arguments &arguments)
{
    *this = arguments;
}

Arguments &Arguments::operator=(const Arguments &arguments)
{
    if (this != &arguments)
    {
        this->arguments = arguments.arguments;
        this->raw = arguments.raw;
        this->argc = arguments.argc;
        this->argv = arguments.argv;
    }
    return (*this);
}

QString Arguments::getConfiguration() const
{
    return (this->arguments.value("configuration"));
}

bool    Arguments::isGui() const
{
    if (this->arguments.contains("nogui"))
        return (false);
    return (true);
}

int     &Arguments::getArgc()
{
    return (this->argc);
}

char    **Arguments::getArgv()
{
    return (this->argv);
}

QString Arguments::toString() const
{
    return (this->raw.join(", ").prepend('"').append('"'));
}

QStringList     Arguments::toStringList() const
{
    QStringList result = this->raw;

    result.removeFirst();
    return (result);
}
