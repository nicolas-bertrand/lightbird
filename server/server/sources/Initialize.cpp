#include "Initialize.h"

Initialize::Initialize(bool initialize) : initialized(initialize)
{
}

Initialize::~Initialize()
{
}

Initialize::Initialize(const Initialize &initialize)
{
    *this = initialize;
}

Initialize &Initialize::operator=(const Initialize &initialize)
{
    if (this != &initialize)
        this->initialized = initialize.initialized;
    return (*this);
}

Initialize::operator bool() const
{
    return (this->initialized);
}

void    Initialize::isInitialized(bool initialized)
{
    this->initialized = initialized;
}
