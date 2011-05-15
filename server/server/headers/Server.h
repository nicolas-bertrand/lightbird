#ifndef SERVER_H
# define SERVER_H

# include <QObject>
# include <QString>

# include "Arguments.h"

/// @brief The main class, called to initialize the server database, configuration,
/// plugins... and which also cleans the server before it quits.
class Server : public QObject
{
    Q_OBJECT

public:
    /// @brief Initialize all the component of the server.
    Server(Arguments &arguments, QObject *parent = 0);
    /// @brief Stop the server. This could take some time, since all the working
    /// thread has to be finished.
    ~Server();

    /// @return True if the server is correctly initialized.
    operator bool();

private:
    Server();
    Server(const Server &server);
    Server  &operator=(const Server &server);

    /// @brief Called by the constructor to initialize the server.
    void    _initialize();
    /// @brief Load the translatations of the texts of the server and the plugins
    /// in the local language.
    /// @param file : The path of the language file on the file system.
    /// @param resource : The path of the language file in the resources. Used only
    /// if the file has not been found using the first parameter.
    /// @return True if the translation has been loaded.
    bool    _loadTranslation(const QString &file, const QString &resource);
    /// @brief Manage the temporary directory. Creates it if it doesn't exists, or
    /// removes its files.
    /// @return True if no error occured.
    bool    _temporaryDirectory();
    /// @brief Load each ports mentionned in the configuration file of the server,
    /// under the nodes <port>, childrens of the node <ports>.
    void    _loadNetwork();
    /// @brief Attemps to load each plugin mentionned in the configuration file of
    /// the server, under the nodes <plugin>, childrens of the node <plugins>.
    void    _loadPlugins();

    Arguments   &arguments;     ///< Stores the arguments used to launch the server.
    bool        initialized;    ///< True if the server has been correctly initialized.

private slots:
    /// @brief Calls the IGui::show() method of a plugin that has just been loaded,
    /// and load its translations if possible.
    void    _pluginLoaded(QString id);
};

#endif // SERVER_H
