/// Environment variables and system information.
// Functions to access environment variables and system information.
// @module environment

#include "Userdata.h"
#include "Environment.h"
#include <Poco/Exception.h>

int luaopen_poco_environment(lua_State* L)
{
    struct LuaPoco::CFunctions methods[] = 
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
    setCFunctions(L, methods);
    
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
    const char* key = luaL_checkstring(L, 1);

    std::string value;
    
    try
    {
        value = Poco::Environment::get(key);
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }

    lua_pushlstring(L, value.c_str(), value.size());
    return 1;
}

/// checks for environment variables existence.
// @string name environment variable's name.
// @return boolean or nil. (error)
// @return error message.
// @function has
int Environment::has(lua_State* L)
{
    const char* key = luaL_checkstring(L, 1);
    bool result = false;
    
    try
    {
        result = Poco::Environment::has(key);
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }

    lua_pushboolean(L, result);
    return 1;
}

/// sets an environment variable's associated value.
// @string name environment variable's name.
// @return boolean or nil. (error)
// @return error message.
// @function set
int Environment::set(lua_State* L)
{
    const char* key = luaL_checkstring(L, 1);
    const char* val = luaL_checkstring(L, 2);
    
    try
    {
        Poco::Environment::set(key, val);
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
        
    lua_pushboolean(L, 1);
    return 1;
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
    std::string value;
    
    try
    {
        value = Poco::Environment::nodeId();
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    lua_pushlstring(L, value.c_str(), value.size());
        
    return 1;
}

/// gets the node (or host) name.
// @return name as a string.
// @function nodeName
int Environment::nodeName(lua_State* L)
{
    std::string value;
    
    try
    {
        value = Poco::Environment::nodeName();
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    lua_pushlstring(L, value.c_str(), value.size());
    return 1;
}

/// gets the operating system architecture.
// @return architecture as a string.
// @function osArchitecture
int Environment::osArchitecture(lua_State* L)
{
    std::string value;
    
    try
    {
        value = Poco::Environment::osArchitecture();
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    lua_pushlstring(L, value.c_str(), value.size());
    return 1;
}

/// gets the operating system name in a "user-friendly" way.
// @return name as a string.
// @function osDisplayName
int Environment::osDisplayName(lua_State* L)
{
    std::string value;
    
    try
    {
        value = Poco::Environment::osDisplayName();
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    lua_pushlstring(L, value.c_str(), value.size());
    return 1;
}

/// gets the operating system name.
// @return name as a string.
// @function osName
int Environment::osName(lua_State* L)
{
    std::string value;
    
    try
    {
        value = Poco::Environment::osName();
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    lua_pushlstring(L, value.c_str(), value.size());
    return 1;
}

/// gets the operating system version.
// @return version as a string.
// @function osVersion
int Environment::osVersion(lua_State* L)
{
    std::string value;
    
    try
    {
        value = Poco::Environment::osVersion();
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    lua_pushlstring(L, value.c_str(), value.size());
    return 1;
}

/// gets the number of processors installed in the system.
// @return count as a number.
// @processorCount
int Environment::processorCount(lua_State* L)
{
    lua_Integer count = Poco::Environment::processorCount();
    lua_pushinteger(L, count);
    return 1;
}

} // LuaPoco
