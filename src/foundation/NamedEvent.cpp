/// Global synchronization object for one process to signal another of an event.
// Note: namedevent is always autoresetting.
//
// Note: There should not be more than one instance of namedevent for a given name in a process. Otherwise, the instances may interfere with each other.
// @module namedevent

#include "NamedEvent.h"
#include <Poco/Exception.h>
#include <cstring>

int luaopen_poco_namedevent(lua_State* L)
{
    LuaPoco::NamedEventUserdata::registerNamedEvent(L);
    return LuaPoco::loadConstructor(L, LuaPoco::NamedEventUserdata::NamedEvent);
}

namespace LuaPoco
{

const char* POCO_NAMEDEVENT_METATABLE_NAME = "Poco.NamedEvent.metatable";

NamedEventUserdata::NamedEventUserdata(const std::string& name) :
    mNamedEvent(name)
{
}

NamedEventUserdata::~NamedEventUserdata()
{
}

// register metatable for this class
bool NamedEventUserdata::registerNamedEvent(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "set", set },
        { "wait", wait },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_NAMEDEVENT_METATABLE_NAME, methods);
    
    return true;
}

/// constructs a new namedevent userdata.
// @string name global event name.
// @return userdata.
// @function new

int NamedEventUserdata::NamedEvent(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    const char* name = luaL_checkstring(L, firstArg);
    NamedEventUserdata* neud = new(lua_newuserdata(L, sizeof *neud)) NamedEventUserdata(name);
    setupPocoUserdata(L, neud, POCO_NAMEDEVENT_METATABLE_NAME);
    return 1;
}

///
// @type namedevent

// metamethod infrastructure
int NamedEventUserdata::metamethod__tostring(lua_State* L)
{
    NamedEventUserdata* neud = checkPrivateUserdata<NamedEventUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.NamedEvent (%p)", static_cast<void*>(neud));
    return 1;
}

// userdata methods

/// Signals the event.
// The process waiting for the event can resume execution.
// @function set
int NamedEventUserdata::set(lua_State* L)
{
    NamedEventUserdata* neud = checkPrivateUserdata<NamedEventUserdata>(L, 1);
    
    neud->mNamedEvent.set();
    
    return 0;
}

/// Waits for the event to become signaled.
// Blocks the process until the event has become signaled.
// @function wait
int NamedEventUserdata::wait(lua_State* L)
{
    NamedEventUserdata* neud = checkPrivateUserdata<NamedEventUserdata>(L, 1);
    
    neud->mNamedEvent.wait();
    
    return 0;
}

} // LuaPoco
