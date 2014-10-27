/// Synchronization mechanism used to control access to a shared resource.
// A semaphore has a value that is constrained to be a non-negative integer and two atomic operations. The allowable operations are V (here called set()) and P (here called wait()). A V (set()) operation increases the value of the semaphore by one. A P (wait()) operation decreases the value of the semaphore by one, provided that can be done without violating the constraint that the value be non-negative. A P (wait()) operation that is initiated when the value of the semaphore is 0 suspends the calling thread. The calling thread may continue when the value becomes positive again. 
//
// Note: semaphore userdata are copyable/sharable between threads.
// @module semaphore

#include "Semaphore.h"
#include "Poco/Exception.h"

int luaopen_poco_semaphore(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::SemaphoreUserdata::Semaphore);
}

namespace LuaPoco
{

SemaphoreUserdata::SemaphoreUserdata(int n) :
    mSemaphore(new Poco::Semaphore(n))
{
}

SemaphoreUserdata::SemaphoreUserdata(int n, int max) :
    mSemaphore(new Poco::Semaphore(n, max))
{
}

// construct new Ud from existing SharedPtr (only useful for 
SemaphoreUserdata::SemaphoreUserdata(const Poco::SharedPtr<Poco::Semaphore>& sem) :
    mSemaphore(sem)
{
}

SemaphoreUserdata::~SemaphoreUserdata()
{
}

bool SemaphoreUserdata::copyToState(lua_State *L)
{
    void* ud = lua_newuserdata(L, sizeof(SemaphoreUserdata));
    luaL_getmetatable(L, "Poco.Semaphore.metatable");
    lua_setmetatable(L, -2);
    
    SemaphoreUserdata* sud = new(ud) SemaphoreUserdata(mSemaphore);
    setPrivateUserdata(L, -1, sud);
    return true;
}

// register metatable for this class
bool SemaphoreUserdata::registerSemaphore(lua_State* L)
{
    luaL_newmetatable(L, "Poco.Semaphore.metatable");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, metamethod__gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, metamethod__tostring);
    lua_setfield(L, -2, "__tostring");
    
    lua_pushstring(L, "Poco.Semaphore.metatable");
    lua_setfield(L, -2, "poco.userdata");
    
    // methods
    lua_pushcfunction(L, set);
    lua_setfield(L, -2, "set");
    lua_pushcfunction(L, tryWait);
    lua_setfield(L, -2, "tryWait");
    lua_pushcfunction(L, wait);
    lua_setfield(L, -2, "wait");
    lua_pop(L, 1);
    
    return true;
}

/// constructs a new semaphore userdata.
// @int n current value of the semaphore.
// @int[opt] max optional maximum value of the semaphore.
// @return userdata or nil. (error)
// @return error message.
// @function new
int SemaphoreUserdata::Semaphore(lua_State* L)
{
    int rv = 0;
    int top = lua_gettop(L);
    int max = 0;
    int n = luaL_checkinteger(L, 1);
    if (top > 1)
        max = luaL_checkinteger(L, 2);

    void* ud = lua_newuserdata(L, sizeof(SemaphoreUserdata));
    luaL_getmetatable(L, "Poco.Semaphore.metatable");
    lua_setmetatable(L, -2);
    try
    {
        rv = 1;
        
        SemaphoreUserdata* sud = NULL;
        if (top > 1)
            sud = new(ud) SemaphoreUserdata(n, max);
        else
            sud = new(ud) SemaphoreUserdata(n);
        
        setPrivateUserdata(L, -1, sud);
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
// @type semaphore
int SemaphoreUserdata::metamethod__tostring(lua_State* L)
{
    SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
        luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
    
    lua_pushfstring(L, "Poco.Semaphore (%p)", reinterpret_cast<void*>(sud));
    return 1;
}

/// Increments the semaphore's value and signals the semaphore. 
// Another thread waiting will continue.
// @function set

// userdata methods
int SemaphoreUserdata::set(lua_State* L)
{
    int rv = 0;
    SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
        luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
    try
    {
        sud->mSemaphore->set();
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        rv = pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Attempts to wait for the semaphore to become signaled.
// To become signaled, a semaphore's value must be greater than zero. Returns true if the semaphore became signaled within the specified time interval, false otherwise. Decrements the semaphore's value by one if successful.
// @int millisecs
// @return boolean
// @function tryWait
int SemaphoreUserdata::tryWait(lua_State* L)
{
    int rv = 0;
    SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
        luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
    long ms = luaL_checkinteger(L, 2);
    
    try
    {
        bool result = sud->mSemaphore->tryWait(ms);
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
/// Waits for the semaphore to become signaled.
// To become signaled, a semaphore's value must be greater than zero. Decrements the semaphore's value by one. 
// @function wait
int SemaphoreUserdata::wait(lua_State* L)
{
    int rv = 0;
    SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
        luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
    
    try
    {
        sud->mSemaphore->wait();
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
