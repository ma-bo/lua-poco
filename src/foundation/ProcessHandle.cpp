/// A userdata handle for a process created with poco.process.launch().
// This handle can be used to determine the process ID of the newly created process and it can be used to wait for the completion of a process.
// @module processhandle

#include "ProcessHandle.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

ProcessHandleUserdata::ProcessHandleUserdata(const Poco::ProcessHandle& ph) :
    mProcessHandle(ph)
{
}

ProcessHandleUserdata::~ProcessHandleUserdata()
{
}

// register metatable for this class
bool ProcessHandleUserdata::registerProcessHandle(lua_State* L)
{
    luaL_newmetatable(L, "Poco.ProcessHandle.metatable");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, metamethod__gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, metamethod__tostring);
    lua_setfield(L, -2, "__tostring");
    
    lua_pushstring(L, "Poco.ProcessHandle.metatable");
    lua_setfield(L, -2, "poco.userdata");
    
    // methods
    lua_pushcfunction(L, id);
    lua_setfield(L, -2, "id");
    lua_pushcfunction(L, wait);
    lua_setfield(L, -2, "wait");
    lua_pushcfunction(L, kill);
    lua_setfield(L, -2, "kill");
    lua_pop(L, 1);

    return true;
}

///
// @type processhandle

// metamethod infrastructure
int ProcessHandleUserdata::metamethod__gc(lua_State* L)
{
    ProcessHandleUserdata* phud = reinterpret_cast<ProcessHandleUserdata*>(
        luaL_checkudata(L, 1, "Poco.ProcessHandle.metatable"));
    phud->~ProcessHandleUserdata();
    
    return 0;
}

int ProcessHandleUserdata::metamethod__tostring(lua_State* L)
{
    int rv = 0;
    ProcessHandleUserdata* phud = reinterpret_cast<ProcessHandleUserdata*>(
        luaL_checkudata(L, 1, "Poco.ProcessHandle.metatable"));
    try
    {
        lua_pushfstring(L, "Poco.ProcessHandle (%p)", reinterpret_cast<void*>(phud));
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

// userdata methods

/// Gets the process id associated with the handle.
// @return id as a Lua number.
// @function id
int ProcessHandleUserdata::id(lua_State* L)
{
    int rv = 0;
    ProcessHandleUserdata* phud = reinterpret_cast<ProcessHandleUserdata*>(
        luaL_checkudata(L, 1, "Poco.ProcessHandle.metatable"));
    try
    {
        lua_pushnumber(L, phud->mProcessHandle.id());
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

/// Waits for the process to terminate and returns the exit code of the process.
// @return exit code as a Lua number.
// @function wait
int ProcessHandleUserdata::wait(lua_State* L)
{
    ProcessHandleUserdata* phud = reinterpret_cast<ProcessHandleUserdata*>(
        luaL_checkudata(L, 1, "Poco.ProcessHandle.metatable"));
    
    lua_pushinteger(L, phud->mProcessHandle.wait());
    
    return 1;
}

/// Kills the process associated with the handle.
// This is preferable on Windows where process IDs may be reused.
// @function kill
int ProcessHandleUserdata::kill(lua_State* L)
{
    ProcessHandleUserdata* phud = reinterpret_cast<ProcessHandleUserdata*>(
        luaL_checkudata(L, 1, "Poco.ProcessHandle.metatable"));
    
    Poco::Process::kill(phud->mProcessHandle);
    
    return 0;
}

} // LuaPoco
