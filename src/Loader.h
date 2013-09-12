#ifndef LUA_POCO_LOADER_H
#define LUA_POCO_LOADER_H

#include "LuaPoco.h"

namespace LuaPoco
{

bool loadMetatables(lua_State* L);
int loadConstructor(lua_State* L, lua_CFunction cons);

} // LuaPoco

#endif
