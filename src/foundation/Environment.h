#ifndef LUA_POCO_ENVIRONMENT_H
#define LUA_POCO_ENVIRONMENT_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/Environment.h"

namespace LuaPoco
{
namespace Environment
{

bool registerEnvironment(lua_State* L);
int get(lua_State* L);
int has(lua_State* L);
int set(lua_State* L);
int libraryVersion(lua_State* L);
int nodeId(lua_State* L);
int nodeName(lua_State* L);
int osArchitecture(lua_State* L);
int osDisplayName(lua_State* L);
int osName(lua_State* L);
int osVersion(lua_State* L);
int processorCount(lua_State* L);

}
} // LuaPoco

#endif
