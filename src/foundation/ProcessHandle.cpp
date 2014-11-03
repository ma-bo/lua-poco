/// A userdata handle for a process created with poco.process.launch().
// This handle can be used to determine the process ID of the newly created process and it can be used to wait for the completion of a process.
// @module processhandle

#include "ProcessHandle.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

const char* POCO_PROCESSHANDLE_METATABLE_NAME = "Poco.ProcessHandle.metatable";

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
    struct UserdataMethod methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "id", id },
        { "wait", wait },
        { "kill", kill },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_PROCESSHANDLE_METATABLE_NAME, methods);
    return true;
}

///
// @type processhandle

// metamethod infrastructure
int ProcessHandleUserdata::metamethod__tostring(lua_State* L)
{
    int rv = 0;
    ProcessHandleUserdata* phud = checkPrivateUserdata<ProcessHandleUserdata>(L, 1);
    
    try
    {
        lua_pushfstring(L, "Poco.ProcessHandle (%p)", static_cast<void*>(phud));
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
    ProcessHandleUserdata* phud = checkPrivateUserdata<ProcessHandleUserdata>(L, 1);
    
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
    ProcessHandleUserdata* phud = checkPrivateUserdata<ProcessHandleUserdata>(L, 1);
    
    lua_pushinteger(L, phud->mProcessHandle.wait());
    
    return 1;
}

/// Kills the process associated with the handle.
// This is preferable on Windows where process IDs may be reused.
// @function kill
int ProcessHandleUserdata::kill(lua_State* L)
{
    ProcessHandleUserdata* phud = checkPrivateUserdata<ProcessHandleUserdata>(L, 1);
    
    Poco::Process::kill(phud->mProcessHandle.id());
    
    return 0;
}

} // LuaPoco
