#ifndef LUA_POCO_STREAMCOPIER_H
#define LUA_POCO_STREAMCOPIER_H

#include "LuaPoco.h"

extern "C"
{
LUAPOCO_API int luaopen_poco_streamcopier(lua_State* L);
}

namespace LuaPoco
{
namespace StreamCopier
{

int copyStream(lua_State* L);
int copyStreamUnbuffered(lua_State* L);
int copyToString (lua_State* L);

} // StreamCopier
} // LuaPoco

#endif
