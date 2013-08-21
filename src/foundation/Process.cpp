#include "Process.h"
#include "ProcessHandle.h"
#include "Pipe.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

bool Process::registerProcess(lua_State* L)
{
	bool result = false;
	if (!lua_istable(L, -1))
		return result;

	lua_createtable(L, 0, 12);
	lua_pushcfunction(L, kill);
	lua_setfield(L, -2, "kill");
	lua_pushcfunction(L, id);
	lua_setfield(L, -2, "id");
	lua_pushcfunction(L, requestTermination);
	lua_setfield(L, -2, "requestTermination");
	lua_pushcfunction(L, times);
	lua_setfield(L, -2, "times");
	lua_pushcfunction(L, launch);
	lua_setfield(L, -2, "launch");
	
	lua_setfield(L, -2, "Process");
	result = true;
	
	return result;
}

int Process::kill(lua_State* L)
{
	int rv = 0;
	try
	{
		Poco::Process::PID pid = luaL_checknumber(L, 1);
		Poco::Process::kill(pid);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

int Process::id(lua_State* L)
{
	int rv = 0;
	try
	{
		lua_Number id = Poco::Process::id();
		lua_pushnumber(L, id);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

int Process::requestTermination(lua_State* L)
{
	int rv = 0;
	try
	{
		Poco::Process::PID pid = luaL_checknumber(L, 1);
		Poco::Process::requestTermination(pid);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

int Process::times(lua_State* L)
{
	int rv = 0;
	try
	{
		long user;
		long kernel;
		
		Poco::Process::times(user, kernel);
		lua_pushinteger(L, user);
		lua_pushinteger(L, kernel);
		rv = 2;
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

namespace
{

// requires that the env table is at -1 on the stack
bool constructEnv(lua_State* L, Poco::Process::Env& env)
{
	int tableIndex = lua_gettop(L);
	
	lua_pushnil(L);
	while (lua_next(L, tableIndex))
	{
		// key and value must be strings
		if (!lua_isstring(L, -1) || lua_isstring(L, -2))
		{
			// restore stack for caller
			lua_pop(L, 2);
			return false;
		}
		
		const char* value = lua_tostring(L, -1);
		const char* key = lua_tostring(L, -2);
		env[key] = value;
		// pop the value, leaving the key at -1 for next()
		lua_pop(L, 1);
	}
	return true;
}

bool constructArgs(lua_State* L, Poco::Process::Args& args)
{
	size_t i = 0;
	// start at index 1
	for (lua_rawgeti(L, ++i, -1); !lua_isnil(L, -1); lua_rawgeti(L, ++i, -1))
	{
		if (!lua_isstring(L, -1))
		{
			// remove value from stack and bail
			lua_pop(L, 1);
			return false;
		}
		
		const char* value = lua_tostring(L, -1);
		args.push_back(value);
		lua_pop(L, 1);
	}
	
	return true;
}

}

int Process::launch(lua_State* L)
{
	int rv = 0;
	luaL_checktype(L, 1, LUA_TTABLE);
	
	// optional table parameters
	bool haveEnv = false;
	const char* workingDir = NULL;
	ProcessHandleUserdata* inPipeUd = NULL;
	ProcessHandleUserdata* outPipeUd = NULL;
	ProcessHandleUserdata* errPipeUd = NULL;
	
	// required parameters
	const char* command = NULL;
	Poco::Process::Args args;
	
	lua_getfield(L, -1, "command");
	if (!lua_isstring(L, -1))
	{
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_pushstring(L, "command table entry must be a string");
		return 2;
	}
	command = lua_tostring(L, -1);
	lua_pop(L, 1);
	
	lua_getfield(L, -1, "args");
	if (lua_istable(L, -1))
	{
		if (!constructArgs(L, args))
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_pushstring(L, "args table entry must be an array of strings");
		return 2;
	}
	lua_pop(L, 1);
	
	try
	{
		
		rv = 0;
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

} // LuaPoco
