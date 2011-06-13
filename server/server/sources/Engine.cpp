#include "Engine.h"

Engine::Engine(Client &c, QObject *parent) : QObject(parent), client(c)
{
    // Initialize the Engine
    this->clear();
}

Engine::~Engine()
{
}

void    Engine::clear()
{
    this->request.clear();
    this->response.clear();
    this->running = false;
    this->done = false;
    this->state = Engine::READY;
    this->protocolUnknow.clear();
}
