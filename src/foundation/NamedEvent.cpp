/// Global synchronization object for one process to signal another of an event.
// Note: namedevent is always autoresetting.
//
// Note: There should not be more than one instance of namedevent for a given name in a process. Otherwise, the instances may interfere with each other.
// @module namedevent

#include "NamedEvent.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_namedevent(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::NamedEventUserdata::NamedEvent);
}

namespace LuaPoco
{

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
    luaL_newmetatable(L, "Poco.NamedEvent.metatable");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, metamethod__gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, metamethod__tostring);
    lua_setfield(L, -2, "__tostring");
    
    // methods
    lua_pushcfunction(L, set);
    lua_setfield(L, -2, "set");
    lua_pushcfunction(L, wait);
    lua_setfield(L, -2, "wait");
    lua_pop(L, 1);
    
    return true;
}

/// constructs a new namedevent userdata.
// @string name global event name.
// @return userdata.
// @function new

int NamedEventUserdata::NamedEvent(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    
    void* ud = lua_newuserdata(L, sizeof(NamedEventUserdata));
    luaL_getmetatable(L, "Poco.NamedEvent.metatable");
    lua_setmetatable(L, -2);
    
    NamedEventUserdata* neud = new(ud) NamedEventUserdata(name);
    setPrivateUserdata(L, -1, neud);
    return 1;
}

///
// @type namedevent

// metamethod infrastructure
int NamedEventUserdata::metamethod__tostring(lua_State* L)
{
    NamedEventUserdata* neud = checkPrivateUserdata<NamedEventUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.NamedEvent (%p)", reinterpret_cast<void*>(neud));
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
