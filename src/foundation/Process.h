#ifndef LUA_POCO_PROCESS_H
#define LUA_POCO_PROCESS_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/Process.h"

namespace LuaPoco
{
namespace Process
{

bool registerProcess(lua_State* L);

int kill(lua_State* L);
int id(lua_State* L);
int launch(lua_State* L);
int requestTermination(lua_State* L);
int times(lua_State* L);

}
} // LuaPoco

#endif
