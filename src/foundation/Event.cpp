/// Synchronization mechanism used to signal one or more other threads.
// Usually, one thread signals an event, while one or more other threads wait for an event to become signaled.
// Note: event userdata are copyable/sharable between threads.
// @module event

#include "Event.h"
#include <Poco/Exception.h>

LUA_API int luaopen_poco_event(lua_State* L)
{
    LuaPoco::EventUserdata::registerEvent(L);
    return LuaPoco::loadConstructor(L, LuaPoco::EventUserdata::Event);
}

namespace LuaPoco
{

const char* POCO_EVENT_METATABLE_NAME = "Poco.Event.metatable";

EventUserdata::EventUserdata(bool autoReset) :
    mEvent(new Poco::Event(autoReset))
{
}

EventUserdata::EventUserdata(const Poco::SharedPtr<Poco::Event>& fm) :
    mEvent(fm)
{
}

EventUserdata::~EventUserdata()
{
}

bool EventUserdata::copyToState(lua_State *L)
{
    registerEvent(L);
    EventUserdata* eud = NULL;
    void* p = lua_newuserdata(L, sizeof *eud);
    
    try
    {
        eud = new(p) EventUserdata(mEvent);
    }
    catch (const std::exception& e)
    {
        lua_pop(L, 1);
        return false;
    }
    
    setupPocoUserdata(L, eud, POCO_EVENT_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool EventUserdata::registerEvent(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "set", set },
        { "tryWait", tryWait },
        { "wait", wait },
        { "reset", reset },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_EVENT_METATABLE_NAME, methods);
    return true;
}

/// create a new event userdata.
// @bool[opt] autoReset optional boolean indicating if the event should auto reset after wait() succesfully returns.
// [default = true]
// @return userdata or nil. (error)
// @return error message.
// @function new
int EventUserdata::Event(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    bool autoReset = true;
    if (lua_isboolean(L, firstArg)) { autoReset = lua_toboolean(L, firstArg); }
    
    EventUserdata* eud = NULL;
    void* p = lua_newuserdata(L, sizeof *eud);
    
    try
    {
        eud = new(p) EventUserdata(autoReset);
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }

    setupPocoUserdata(L, eud, POCO_EVENT_METATABLE_NAME);
    return 1;
}

///
// @type event

// metamethod infrastructure
int EventUserdata::metamethod__tostring(lua_State* L)
{
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.Event (%p)", static_cast<void*>(eud));
    return 1;
}

// userdata methods

/// Signals the event. 
// If autoReset is true, only one thread waiting for the event can resume execution. If autoReset is false, all waiting threads can resume execution. 
// @function set
int EventUserdata::set(lua_State* L)
{
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1);
    
    try
    {
        eud->mEvent->set();
    }
    catch (const std::exception& e)
    {
        pushException(L, e);
        lua_error(L);
    }
        
    return 0;
}

/// Attempts to wait for the event.
// @int ms number of milliseconds to try to wait for the event.
// @return boolean returning true if event was signaled or false if a timeout occured.
// @function tryWait
int EventUserdata::tryWait(lua_State* L)
{
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1); 
    long ms = luaL_checkinteger(L, 2);
    bool result = false;
    
    try
    {
        result = eud->mEvent->tryWait(ms);
    }
    catch (const std::exception& e)
    {
        pushException(L, e);
        lua_error(L);
    }
    
    lua_pushboolean(L, result);
    return 1;
}

/// Waits for the event to become signaled.
// @function wait
int EventUserdata::wait(lua_State* L)
{
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1);
    
    try
    {
        eud->mEvent->wait();
    }
    catch (const std::exception& e)
    {
        pushException(L, e);
        lua_error(L);
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

/// Resets the event to unsignaled state. 
// @function reset
int EventUserdata::reset(lua_State* L)
{
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1);
    
    try
    {
        eud->mEvent->reset();
    }
    catch (const std::exception& e)
    {
        pushException(L, e);
        lua_error(L);
    }
    
    return 0;
}

} // LuaPoco
