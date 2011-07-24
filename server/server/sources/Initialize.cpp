#include "Initialize.h"

Initialize::Initialize(bool initialize) : initialized(initialize)
{
}

Initialize::~Initialize()
{
}

Initialize::operator bool() const
{
    return (this->initialized);
}

void    Initialize::isInitialized(bool initialized)
{
    this->initialized = initialized;
}
