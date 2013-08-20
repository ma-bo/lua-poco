#include "Environment.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

bool Environment::registerEnvironment(lua_State* L)
{
	bool result = false;
	if (!lua_istable(L, -1))
		return result;

	lua_createtable(L, 0, 12);
	lua_pushcfunction(L, get);
	lua_setfield(L, -2, "get");
	lua_pushcfunction(L, has);
	lua_setfield(L, -2, "has");
	lua_pushcfunction(L, set);
	lua_setfield(L, -2, "set");
	lua_pushcfunction(L, libraryVersion);
	lua_setfield(L, -2, "libraryVersion");
	lua_pushcfunction(L, nodeId);
	lua_setfield(L, -2, "nodeId");
	lua_pushcfunction(L, nodeName);
	lua_setfield(L, -2, "nodeName");
	lua_pushcfunction(L, osArchitecture);
	lua_setfield(L, -2, "osArchitecture");
	lua_pushcfunction(L, osDisplayName);
	lua_setfield(L, -2, "osDisplayName");
	lua_pushcfunction(L, osName);
	lua_setfield(L, -2, "osName");
	lua_pushcfunction(L, osVersion);
	lua_setfield(L, -2, "osVersion");
	lua_pushcfunction(L, processorCount);
	lua_setfield(L, -2, "processorCount");
	
	lua_setfield(L, -2, "Environment");
	result = true;
	
	return result;
}

int Environment::get(lua_State* L)
{
	int rv = 0;
	try
	{
		const char* key = luaL_checkstring(L, 1);
		std::string value = Poco::Environment::get(key);
		lua_pushlstring(L, value.c_str(), value.size());
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

int Environment::has(lua_State* L)
{
	int rv = 0;
	try
	{
		const char* key = luaL_checkstring(L, 1);
		bool result = Poco::Environment::has(key);
		lua_pushboolean(L, result);
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

int Environment::set(lua_State* L)
{
	int rv = 0;
	try
	{
		const char* key = luaL_checkstring(L, 1);
		const char* val = luaL_checkstring(L, 2);
		Poco::Environment::set(key, val);
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

int Environment::libraryVersion(lua_State* L)
{
	Poco::Int32 ver = Poco::Environment::libraryVersion();
	lua_pushinteger(L, ver);
	return 1;
}

int Environment::nodeId(lua_State* L)
{
	int rv = 0;
	try
	{
		std::string value = Poco::Environment::nodeId();
		lua_pushlstring(L, value.c_str(), value.size());
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

int Environment::nodeName(lua_State* L)
{
	int rv = 0;
	try
	{
		std::string value = Poco::Environment::nodeName();
		lua_pushlstring(L, value.c_str(), value.size());
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

int Environment::osArchitecture(lua_State* L)
{
	int rv = 0;
	try
	{
		std::string value = Poco::Environment::osArchitecture();
		lua_pushlstring(L, value.c_str(), value.size());
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

int Environment::osDisplayName(lua_State* L)
{
	int rv = 0;
	try
	{
		std::string value = Poco::Environment::osDisplayName();
		lua_pushlstring(L, value.c_str(), value.size());
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

int Environment::osName(lua_State* L)
{
	int rv = 0;
	try
	{
		std::string value = Poco::Environment::osName();
		lua_pushlstring(L, value.c_str(), value.size());
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

int Environment::osVersion(lua_State* L)
{
	int rv = 0;
	try
	{
		std::string value = Poco::Environment::osVersion();
		lua_pushlstring(L, value.c_str(), value.size());
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

int Environment::processorCount(lua_State* L)
{
	lua_Number count = Poco::Environment::processorCount();
	lua_pushnumber(L, count);
	return 1;
}

} // LuaPoco
