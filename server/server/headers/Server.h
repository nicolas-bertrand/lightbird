#ifndef SERVER_H
# define SERVER_H

# include <QSplashScreen>
# include <QObject>
# include <QString>
# include <QStringList>

/// @brief The main class, called to initialize the server database, configuration,
/// plugins, etc. and which also cleans the server before it quits.
class Server : public QObject
{
    Q_OBJECT

public:
    /// @brief Initialize the server. Use isInitialized to see if an error occured.
    Server(int argc, char *argv[], QObject *parent = 0);
    /// @brief Stop the server. This could take some time, since all the working thread has to be finished.
    ~Server();

    /// @return True if the server is correctly initialized.
    bool    isInitialized();
    /// @brief Runs all the unit tests of the server.
    /// @return False is at least one test failed.
    bool    unitTests();

private:
    Server();
    Server(const Server &server);
    Server  &operator=(const Server &server);

    /// @brief Called by the constructors, this method initialize the components of the server.
    /// If the initialization is successful, the variable initialized is set at true.
    void    _initialize();
    /// @brief Show the splash screen if it is hide, and hide it if it is displayed. It is displayed
    /// only in gui mode.
    /// @param path : The path of the image displayed in the splash screen.
    void    _splashScreen(const QString &path = "");
    /// @brief Load the translatations of the texts of the server, in the local language.
    bool    _loadTranslation(const QString &resource, const QString &file);
    /// @brief Manage the temporary directory. Creates it if it doesn't exists, or removes its files.
    bool    _temporaryDirectory();
    /// @brief Load each ports mentionned in the configuration file of the server, under the nodes
    /// <port>, childrens of the node <ports>.
    void    _loadNetwork();
    /// @brief Attemps to load each plugin mentionned in the configuration file of the server,
    /// under the nodes <plugin>, childrens of the node <plugins>.
    void    _loadPlugins();

    QStringList     arguments;          /// The list of the arguments of the program (from argc and argv).
    QString         configurationPath;  /// The path to the XML configuration file of the server.
    bool            initialized;        /// True if the server has been correctly initialized by _initialize().
    QSplashScreen   *splashScreen;      /// An image displaying while the server in loading.

private slots:
    /// @brief Calls the IGui::show() method of a plugin that has just been loaded,
    /// and load its translations if possible.
    void    _pluginLoaded(QString id);
};

#endif // SERVER_H
