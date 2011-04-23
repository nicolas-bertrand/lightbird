#include <iostream>
#include <QApplication>

#include "Arguments.h"
#include "Server.h"
#include "Log.h"

QCoreApplication        *loadQt(Arguments &arguments);

int                     main(int argc, char **argv)
{
    Arguments           arguments(argc, argv);
    QCoreApplication    *application;
    Server              *server;
    int                 result;

    // Instantiales Qt core class
    application = loadQt(arguments);
    // Instanciates and initializes the server
    if (*(server = new Server(arguments, application)) == false)
    {
        // If the initialization of the server failed displays the logs on the
        // standard output, in case the error occured before the logs was initialized.
        Log::instance()->print();
        std::cerr << "An error occurred while initializing the server. Check the log entries for more information." << std::endl;
        delete server;
        return (1);
    }
    Log::info("Executing the main event loop", "Server", "_initialize");
    result = application->exec();
    Log::info("The main event loop has finished", Properties("code", QString::number(result)), "main.cpp", "main");
    delete server;
    delete application;
    return (result);
}

/// @brief Instanciate the main class of Qt. If the argument "-noGui" is defined
/// in the command line, QCoreApplication is created. Otherwise QApplication is
/// instanciates. QApplication allows the server to display GUIs feature. In contrast,
/// QCoreApplication run the server as a console application, without any GUIs.
QCoreApplication        *loadQt(Arguments &arguments)
{
    QCoreApplication    *application;

    if (arguments.isGui())
        application = new QApplication(arguments.getArgc(), arguments.getArgv());
    else
        application = new QCoreApplication(arguments.getArgc(), arguments.getArgv());
    return (application);
}
