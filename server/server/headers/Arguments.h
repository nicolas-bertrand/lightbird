#ifndef ARGUMENTS_H
# define ARGUMENTS_H

#include <QMap>
#include <QString>
#include <QStringList>

/// @brief Parse and store the arguments given to the server.
class Arguments
{
public:
    Arguments();
    /// @brief Parses the arguments in parameter.
    Arguments(int argc, char **argv);
    ~Arguments();
    Arguments(const Arguments &arguments);
    Arguments &operator=(const Arguments &arguments);

    /// @brief Returns the configuration path of the server.
    QString     getConfiguration();
    /// @brief Returns true if the server can display GUIs.
    bool        isGui();
    /// @brief Returns argc.
    int         &getArgc();
    /// @brief Returns argv.
    char        **getArgv();
    /// @brief Returns a string that contains all the arguments of the server.
    QString     toString();
    /// @brief Returns a string list that contains all the arguments except the server executable.
    QStringList toStringList();

private:
    QMap<QString, QString>  arguments;  ///< The arguments after they have been parsed.
    QStringList             raw;        ///< The raw list of arguments (from argv).
    int                     argc;       ///< The original argc.
    char                    **argv;     ///< The original argv.
};

#endif // ARGUMENTS_H
