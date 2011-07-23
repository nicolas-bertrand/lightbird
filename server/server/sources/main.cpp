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
    int                 result;

    // Instanciates Qt core class
    application = ::loadQt(arguments);
    // Instanciates and initializes the server
    if (Server::instance(arguments, application))
    {
        Log::info("Executing the main event loop", "Server", "_initialize");
        result = application->exec();
        Log::info("The main event loop has finished", Properties("code", QString::number(result)), "main.cpp", "main");
    }
    // An error occured during the initialization
    else
    {
        Log::instance()->print();
        Log::instance()->isDisplay(true);
        std::cerr << "An error occurred while initializing the server. Check the log entries for more information." << std::endl;
        result = 1;
    }
    Server::shutdown();
    delete application;
    while (true)
        ;
    return (result);
}

/// @brief Instanciates the main class of Qt. If the argument "-noGui" is defined
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
