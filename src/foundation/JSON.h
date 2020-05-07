#ifndef LUA_POCO_JSON_H
#define LUA_POCO_JSON_H

#include "LuaPoco.h"

extern "C"
{
LUAPOCO_API int luaopen_poco_json(lua_State* L);
}

namespace LuaPoco
{
namespace JSON
{

int decode(lua_State* L);
int encode(lua_State* L);
int getNull(lua_State* L);
int getEmptyObject(lua_State* L);
int getEmptyArray(lua_State* L);

}
} // LuaPoco

#endif
