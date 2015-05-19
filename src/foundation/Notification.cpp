#include "Loader.h"
#include "Notification.h"

namespace LuaPoco
{

Notification::Notification()
{
    state = luaL_newstate();
}

Notification::~Notification()
{
    lua_close(state);
}

bool Notification::setupState()
{
    return loadMetatables(state);
}

} // LuaPoco
