#include "Arguments.h"

Arguments::Arguments(int ac, char **av) : argc(ac),
                                          argv(av)
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

QString Arguments::getConfiguration()
{
    return (this->arguments.value("configuration"));
}

bool    Arguments::isGui()
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

QString Arguments::toString()
{
    return (this->raw.join(", ").prepend('"').append('"'));
}

QStringList     Arguments::toStringList()
{
    QStringList result = this->raw;

    result.removeFirst();
    return (result);
}
