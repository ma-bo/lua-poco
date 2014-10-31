/// Environment variables and system information.
// Functions to access environment variables and system information.
// @module env

#include "Userdata.h"
#include "Environment.h"
#include "Poco/Exception.h"

int luaopen_poco_env(lua_State* L)
{
    struct LuaPoco::UserdataMethod methods[] = 
    {
        { "get", LuaPoco::Environment::get },
        { "has", LuaPoco::Environment::has },
        { "set", LuaPoco::Environment::set },
        { "libraryVersion", LuaPoco::Environment::libraryVersion },
        { "nodeId", LuaPoco::Environment::nodeId },
        { "nodeName", LuaPoco::Environment::nodeName },
        { "osArchitecture", LuaPoco::Environment::osArchitecture },
        { "osDisplayName", LuaPoco::Environment::osDisplayName },
        { "osName", LuaPoco::Environment::osName },
        { "osVersion", LuaPoco::Environment::osVersion },
        { "processorCount", LuaPoco::Environment::processorCount },
        { NULL, NULL}
    };
    
    lua_createtable(L, 0, 11);
    setMetatableFunctions(L, methods);
    
    return 1;
}

namespace LuaPoco
{

/// gets an environment variable's associated value.
// @string name environment variable's name.
// @return value as string or nil. (error)
// @return error message.
// @function get
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

/// checks for environment variables existence.
// @string name environment variable's name.
// @return boolean or nil. (error)
// @return error message.
// @function has
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

/// sets an environment variable's associated value.
// @string name environment variable's name.
// @return boolean or nil. (error)
// @return error message.
// @function set
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

/// gets the POCO C++ library version. 
// number is in the format of 0xAABBCCDD, 
// where AA is the major version number, BB is the minor version number, CC is the revision number,
// DD is the patch level number.
// @return integer representing underlying POCO library version.
// @function libraryVersion
int Environment::libraryVersion(lua_State* L)
{
    Poco::Int32 ver = Poco::Environment::libraryVersion();
    lua_pushinteger(L, ver);
    return 1;
}
/// gets the ethernet address of the first ethernet adapter found on the system.
// @return string of bytes representing MAC address.
// @function nodeId
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

/// gets the node (or host) name.
// @return name as a string.
// @function nodeName
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

/// gets the operating system architecture.
// @return architecture as a string.
// @function osArchitecture
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

/// gets the operating system name in a "user-friendly" way.
// @return name as a string.
// @function osDisplayName
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

/// gets the operating system name.
// @return name as a string.
// @function osName
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

/// gets the operating system version.
// @return version as a string.
// @function osVersion
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

/// gets the number of processors installed in the system.
// @return count as a number.
// @processorCount
int Environment::processorCount(lua_State* L)
{
    lua_Number count = Poco::Environment::processorCount();
    lua_pushnumber(L, count);
    return 1;
}

} // LuaPoco
