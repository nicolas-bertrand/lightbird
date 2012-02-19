#include "Library.h"

LightBird::ILogs *LightBird::Library::logs = NULL;

LightBird::ILogs &LightBird::Library::log()
{
    return (*LightBird::Library::logs);
}

void LightBird::Library::setLogs(LightBird::ILogs *logs)
{
    if (!LightBird::Library::logs)
        LightBird::Library::logs = logs;
}
