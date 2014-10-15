#include "ClientsNetwork.h"
#ifdef Q_OS_WIN
# include "ClientsNetworkWindows.h"
#else
# include "ClientsLinux.h"
#endif // Q_OS_WIN

ClientsNetwork *ClientsNetwork::create()
{
    ClientsNetwork *clientsNetwork = NULL;

#ifdef Q_OS_WIN
    clientsNetwork = new ClientsNetworkWindows();
#else
    clientsNetwork = new ClientsLinux();
#endif // Q_OS_WIN

    if (clientsNetwork && !*clientsNetwork)
    {
        delete clientsNetwork;
        clientsNetwork = NULL;
    }
    return clientsNetwork;
}

ClientsNetwork::ClientsNetwork()
    : _listening(false)
{
}

ClientsNetwork::~ClientsNetwork()
{
}
