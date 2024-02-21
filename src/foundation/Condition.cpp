/// A synchronization object used to block a thread until a particular condition is met.
// @module condition

#include "Condition.h"
#include "FastMutex.h"
#include "Mutex.h"
#include <Poco/Exception.h>

int luaopen_poco_condition(lua_State* L)
{
    LuaPoco::ConditionUserdata::registerCondition(L);
    return LuaPoco::loadConstructor(L, LuaPoco::ConditionUserdata::Condition);
}

namespace LuaPoco
{

const char* POCO_CONDITION_METATABLE_NAME = "Poco.Condition.metatable";

ConditionUserdata::ConditionUserdata() :
    mCondition(new Poco::Condition())
{
}

ConditionUserdata::ConditionUserdata(const Poco::SharedPtr<Poco::Condition>& condition) :
    mCondition(condition)
{
}

ConditionUserdata::~ConditionUserdata()
{
}

bool ConditionUserdata::copyToState(lua_State *L)
{
    registerCondition(L);
    ConditionUserdata* cud = new(lua_newuserdata(L, sizeof *cud)) ConditionUserdata(mCondition);
    setupPocoUserdata(L, cud, POCO_CONDITION_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool ConditionUserdata::registerCondition(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "broadcast",  broadcast },
        { "signal",  signal },
        { "wait", wait },
        { "tryWait", tryWait },
        { NULL, NULL}
    };

    setupUserdataMetatable(L, POCO_CONDITION_METATABLE_NAME, methods);
    return true;
}

/// create a new condition userdata.
// @return userdata or nil. (error)
// @return error message.
// @function new
int ConditionUserdata::Condition(lua_State* L)
{
    int rv = 0;

    try
    {
        ConditionUserdata* cud = new(lua_newuserdata(L, sizeof *cud)) ConditionUserdata();
        setupPocoUserdata(L, cud, POCO_CONDITION_METATABLE_NAME);
        rv = 1;
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
    
    return rv;
}

///
// @type condition

// metamethod infrastructure
int ConditionUserdata::metamethod__tostring(lua_State* L)
{
    ConditionUserdata* cud = checkPrivateUserdata<ConditionUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.Condition (%p)", static_cast<void*>(cud));
    return 1;
}

// userdata methods

/// Signals the Condition and allows all waiting threads to continue their execution.
// @function broadcast
int ConditionUserdata::broadcast(lua_State* L)
{
    int rv = 0;
    ConditionUserdata* cud = checkPrivateUserdata<ConditionUserdata>(L, 1);
    
    try
    {
        cud->mCondition->broadcast();
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
        lua_error(L);
    }
    
    return rv;
}

/// Signals the condition and allows one waiting thread to continue execution.
// @function signal
int ConditionUserdata::signal(lua_State* L)
{
    int rv = 0;
    ConditionUserdata* cud = checkPrivateUserdata<ConditionUserdata>(L, 1);
    
    cud->mCondition->signal();
    return rv;
}

/// Unlocks the mutex and waits until the condition is signaled.
// The mutex must be locked before calling wait().
// @tparam userdata mutex either fastmutex or mutex userdata that must be in the locked state prior to calling.
// @function wait
int ConditionUserdata::wait(lua_State* L)
{
    int rv = 0;
    ConditionUserdata* cud = checkPrivateUserdata<ConditionUserdata>(L, 1);
    Userdata* ud = checkPrivateUserdata<Userdata>(L, 2);

    FastMutexUserdata* fud = dynamic_cast<FastMutexUserdata*>(ud);
    MutexUserdata* mud = dynamic_cast<MutexUserdata*>(ud);

    if (fud == NULL && mud == NULL)
    {
        lua_pushnil(L);
        lua_pushstring(L, "expecting fastmutex or mutex userdata.");
        return lua_error(L);
    }
    
    if (fud) { cud->mCondition->wait(*fud->mFastMutex); }
    else if (mud) { cud->mCondition->wait(*mud->mMutex); }

    return 0;
}

/// Unlocks the mutex and waits for the given time until the condition is signaled.
// The mutex must be locked before calling tryWait().
// The given mutex will be locked again upon leaving the function.
// returns true if the condition has been signaled within the given time interval, otherwise false.
// @tparam userdata mutex either fastmutex or mutex userdata that must be in the locked state prior to calling.
// @function tryWait
int ConditionUserdata::tryWait(lua_State* L)
{
    int rv = 0;
    ConditionUserdata* cud = checkPrivateUserdata<ConditionUserdata>(L, 1);
    Userdata* ud = checkPrivateUserdata<Userdata>(L, 2);
    long ms = 0;
    
    if (lua_isnumber(L, 3)) { ms = static_cast<long>(lua_tointeger(L, 3)); }

    FastMutexUserdata* fud = dynamic_cast<FastMutexUserdata*>(ud);
    MutexUserdata* mud = dynamic_cast<MutexUserdata*>(ud);

    if (fud == NULL && mud == NULL)
    {
        lua_pushnil(L);
        lua_pushstring(L, "expecting fastmutex or mutex userdata.");
        return lua_error(L);
    }
    
    bool result = false;
    if (fud) { result = cud->mCondition->tryWait(*fud->mFastMutex, ms); }
    else if (mud) { result = cud->mCondition->tryWait(*mud->mMutex, ms); }

    lua_pushboolean(L, static_cast<int>(result));
    return 1;
}

} // LuaPoco
