#include <iostream>
#include <QApplication>

#include "Server.h"
#include "Log.h"

QCoreApplication        *createQApplication(int argc, char *argv[]);

int                     main(int argc, char *argv[])
{
    QCoreApplication    *app;
    Server              *server;
    int                 result;

    // Instantiales Qt core class
    app = createQApplication(argc, argv);
    // Instanciates and initializes the server
    server = new Server(argc, argv, app);
    // Check if the server has been initialized
    if (server->isInitialized() == false)
    {
        // If the initialization of the server failed
        // Display the log on the standard output if the error occured before the logs was initialized
        Log::instance()->print();
        std::cerr << "An error occurred while initializing the server. Check the log entries for more information." << std::endl;
        delete server;
        return (1);
    }
    Log::info("Executing the main event loop", "Server", "_initialize");
    result = app->exec();
    Log::info("The main event loop has finished", Properties("code", QString::number(result)), "Server", "_initialize");
    delete server;
    delete app;
    return (result);
}

/// @brief Instanciate the main class of Qt. If the argument "-noGui" is defined
/// in the command line, QCoreApplication is created. Otherwise QApplication is
/// instanciates. QApplication allows the server to display GUIs feature. In contrast,
/// QCoreApplication run the server as a console application, without any GUIs.
QCoreApplication        *createQApplication(int argc, char *argv[])
{
    QCoreApplication    *app;
    bool                gui;

    gui = true;
    for (int i = 1; i < argc && gui == true; ++i)
        if (QString(argv[i]).toLower() == "-nogui")
            gui = false;
    if (gui)
        app = new QApplication(argc, argv);
    else
        app = new QCoreApplication(argc, argv);
    return (app);
}
