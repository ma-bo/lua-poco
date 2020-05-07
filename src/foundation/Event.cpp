/// Synchronization mechanism used to signal one or more other threads.
// Usually, one thread signals an event, while one or more other threads wait for an event to become signaled.
// Note: event userdata are copyable/sharable between threads.
// @module event

#include "Event.h"
#include "Poco/Exception.h"

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
    EventUserdata* eud = new(lua_newuserdata(L, sizeof *eud)) EventUserdata(mEvent);
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
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    bool autoReset = true;
    if (lua_gettop(L) > firstArg)
    {
        luaL_checktype(L, firstArg, LUA_TBOOLEAN);
        autoReset = lua_toboolean(L, firstArg);
    }
    
    try
    {
        EventUserdata* eud = new(lua_newuserdata(L, sizeof *eud)) EventUserdata(autoReset);
        setupPocoUserdata(L, eud, POCO_EVENT_METATABLE_NAME);
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
    int rv = 0;
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1);
    
    try
    {
        eud->mEvent->set();
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Attempts to wait for the event.
// @int ms number of milliseconds to try to wait for the event.
// @return boolean returning true if event was signaled or false if a timeout occured.
// @function tryWait
int EventUserdata::tryWait(lua_State* L)
{
    int rv = 0;
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1);
    
    long ms = luaL_checkinteger(L, 2);
    
    try
    {
        bool result = false;
        result = eud->mEvent->tryWait(ms);
        
        lua_pushboolean(L, result);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Waits for the event to become signaled.
// @function wait
int EventUserdata::wait(lua_State* L)
{
    int rv = 0;
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1);
    
    try
    {
        eud->mEvent->wait();
        lua_pushboolean(L, 1);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Resets the event to unsignaled state. 
// @function reset
int EventUserdata::reset(lua_State* L)
{
    int rv = 0;
    EventUserdata* eud = checkPrivateUserdata<EventUserdata>(L, 1);
    
    try
    {
        eud->mEvent->reset();
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

} // LuaPoco
