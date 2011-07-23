#include "Initialize.h"

Initialize::Initialize(bool initialize) : initialized(initialize)
{
}

Initialize::~Initialize()
{
}

Initialize::operator bool()
{
    return (this->initialized);
}

void    Initialize::isInitialized(bool initialized)
{
    this->initialized = initialized;
}
