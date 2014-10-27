/// Timestamps.
// Stores a monotonic* time value with (theoretical) microseconds resolution. Timestamps can be compared with each other and simple arithmetic is supported.
// Note: Timestamp values are only monotonic as long as the systems's clock is monotonic as well (and not, e.g. set back).
// Note: Timestamps are UTC (Coordinated Universal Time) based.
// @module timestamp

#include "Timestamp.h"
#include "DynamicAny.h"
#include <ctime>

int luaopen_poco_timestamp(lua_State* L)
{
    int rv = 0;
    
    if (LuaPoco::loadMetatables(L))
    {
        lua_createtable(L, 0, 1);
        lua_pushcfunction(L, LuaPoco::TimestampUserdata::Timestamp);
        lua_setfield(L, -2, "new");
        lua_pushcfunction(L, LuaPoco::TimestampUserdata::TimestampFromEpoch);
        lua_setfield(L, -2, "fromEpoch");
        lua_pushcfunction(L, LuaPoco::TimestampUserdata::TimestampFromUtc);
        lua_setfield(L, -2, "fromUTC");
        rv = 1;
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, "failed to create required poco metatables");
        rv = 2;
    }
    
    return rv;
}

namespace LuaPoco
{

TimestampUserdata::TimestampUserdata() :
    mTimestamp()
{
}

TimestampUserdata::TimestampUserdata(Poco::Int64 tv) :
    mTimestamp(tv)
{
}

TimestampUserdata::TimestampUserdata(const Poco::Timestamp& ts) :
    mTimestamp(ts)
{
}

TimestampUserdata::~TimestampUserdata()
{
}

bool TimestampUserdata::copyToState(lua_State* L)
{
    void* ud = lua_newuserdata(L, sizeof *this);
    luaL_getmetatable(L, "Poco.Timestamp.metatable");
    lua_setmetatable(L, -2);
    TimestampUserdata* tsud = new(ud) TimestampUserdata(mTimestamp);
    setPrivateUserdata(L, -1, tsud);
    
    return true;
}

// register metatable for this class
bool TimestampUserdata::registerTimestamp(lua_State* L)
{
    luaL_newmetatable(L, "Poco.Timestamp.metatable");
    // metamethods
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, metamethod__gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, metamethod__tostring);
    lua_setfield(L, -2, "__tostring");
    
    lua_pushstring(L, "Poco.Timestamp.metatable");
    lua_setfield(L, -2, "poco.userdata");
    
    // Timestamp methods
    lua_pushcfunction(L, elapsed);
    lua_setfield(L, -2, "elapsed");
    lua_pushcfunction(L, epochMicroseconds);
    lua_setfield(L, -2, "epochMicroseconds");
    lua_pushcfunction(L, epochTime);
    lua_setfield(L, -2, "epochTime");
    lua_pushcfunction(L, isElapsed);
    lua_setfield(L, -2, "isElapsed");
    lua_pushcfunction(L, update);
    lua_setfield(L, -2, "update");
    lua_pushcfunction(L, utcTime);
    lua_setfield(L, -2, "utcTime");
    lua_pushcfunction(L, metamethod__add);
    lua_setfield(L, -2, "__add");
    lua_pushcfunction(L, metamethod__sub);
    lua_setfield(L, -2, "__sub");
    lua_pushcfunction(L, metamethod__eq);
    lua_setfield(L, -2, "__eq");
    lua_pushcfunction(L, metamethod__lt);
    lua_setfield(L, -2, "__lt");
    lua_pushcfunction(L, metamethod__le);
    lua_setfield(L, -2, "__le");
    lua_pop(L, 1);
    
    return true;
}

// standalone functions that create a TimestampUserdata

/// Constructs a new timestamp userdata from epoch value.
// @int epoch value used to create timestamp.
// @return userdata or nil. (error)
// @return error message.
// @function fromEpoch
int TimestampUserdata::TimestampFromEpoch(lua_State* L)
{
    int rv = 0;
    std::time_t val = luaL_checkinteger(L, 1);
    try
    {
        void *ud = lua_newuserdata(L, sizeof(TimestampUserdata));
        luaL_getmetatable(L, "Poco.Timestamp.metatable");
        lua_setmetatable(L, -2);
        TimestampUserdata* tsud = new (ud) TimestampUserdata(Poco::Timestamp::fromEpochTime(val));
        setPrivateUserdata(L, -1, tsud);
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

/// Constructs a new timestamp userdata from UTC value.
// @int utc value used to create timestamp.
// @return userdata or nil. (error)
// @return error message.
// @function fromEpoch
int TimestampUserdata::TimestampFromUtc(lua_State* L)
{
    int rv = 0;
    
    Poco::Int64 val = 0;
    DynamicAnyUserdata* daud = checkPrivateUserdata<DynamicAnyUserdata>(L, 1);
    
    try
    {
        daud->mDynamicAny.convert(val);
        void *ud = lua_newuserdata(L, sizeof(TimestampUserdata));
        luaL_getmetatable(L, "Poco.Timestamp.metatable");
        lua_setmetatable(L, -2);
        TimestampUserdata* tsud = new (ud) TimestampUserdata(val);
        setPrivateUserdata(L, -1, tsud);
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

// Lua constructor

/// Constructs a new timestamp userdata from current time.
// @return userdata or nil. (error)
// @return error message.
// @function new
int TimestampUserdata::Timestamp(lua_State* L)
{
    int rv = 0;
    int top = lua_gettop(L);
    
    try
    {
        void *ud = lua_newuserdata(L, sizeof(TimestampUserdata));
        luaL_getmetatable(L, "Poco.Timestamp.metatable");
        lua_setmetatable(L, -2);
        TimestampUserdata* tsud = new (ud) TimestampUserdata();
        setPrivateUserdata(L, -1, tsud);
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
// @type timestamp

int TimestampUserdata::metamethod__tostring(lua_State* L)
{
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    lua_pushfstring(L, "Poco.Timestamp (%p)", reinterpret_cast<void*>(tsud));
    return 1;
}

int TimestampUserdata::metamethod__add(lua_State* L)
{
    int rv = 0;
    int tsIndex = 1;
    int otherIndex = 2;
    
    if (!lua_isuserdata(L, 1) || !lua_isuserdata(L, 2))
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Poco.Timestamp and Poco.DynamicAny required for __add");
        return 2;
    }

    lua_getmetatable(L, 1);
    luaL_getmetatable(L, "Poco.Timestamp.metatable");
    if (!lua_rawequal(L, -1, -2))
    {
        tsIndex = 2;
        otherIndex = 1;
    }
    lua_pop(L, 2);
    
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, tsIndex);
    DynamicAnyUserdata* daud = checkPrivateUserdata<DynamicAnyUserdata>(L, otherIndex);
    
    try
    {
        Poco::Int64 val;
        daud->mDynamicAny.convert(val);
        Poco::Timestamp newTs;
        
        newTs = tsud->mTimestamp + val;
        
        void *ud = lua_newuserdata(L, sizeof(TimestampUserdata));
        luaL_getmetatable(L, "Poco.Timestamp.metatable");
        lua_setmetatable(L, -2);
        TimestampUserdata* tsudNew = new (ud) TimestampUserdata(newTs);
        setPrivateUserdata(L, -1, tsudNew);
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

int TimestampUserdata::metamethod__sub(lua_State* L)
{
    int rv = 0;
    int tsIndex = 1;
    int otherIndex = 2;
    
    if (!lua_isuserdata(L, 1) || !lua_isuserdata(L, 2))
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Poco.Timestamp and Poco.DynamicAny required for __sub");
        return 2;
    }

    lua_getmetatable(L, 1);
    luaL_getmetatable(L, "Poco.Timestamp.metatable");
    if (!lua_rawequal(L, -1, -2))
    {
        tsIndex = 2;
        otherIndex = 1;
    }
    lua_pop(L, 2);
    
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, tsIndex);
    DynamicAnyUserdata* daud = checkPrivateUserdata<DynamicAnyUserdata>(L, otherIndex);
    
    try
    {
        Poco::Int64 val;
        daud->mDynamicAny.convert(val);
        Poco::Timestamp newTs;
        
        if (tsIndex == 1)
            newTs = tsud->mTimestamp - val;
        else
            newTs = val - tsud->mTimestamp;
        
        void* ud = lua_newuserdata(L, sizeof(TimestampUserdata));
        luaL_getmetatable(L, "Poco.Timestamp.metatable");
        lua_setmetatable(L, -2);
        TimestampUserdata* tsudNew = new (ud) TimestampUserdata(newTs);
        setPrivateUserdata(L, -1, tsudNew);
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

int TimestampUserdata::metamethod__lt(lua_State* L)
{
    TimestampUserdata* tsudLhs = checkPrivateUserdata<TimestampUserdata>(L, 1);
    TimestampUserdata* tsudRhs = checkPrivateUserdata<TimestampUserdata>(L, 2);
    
    bool result = tsudLhs->mTimestamp < tsudRhs->mTimestamp;
    lua_pushboolean(L, result);
    
    return 1;
}

int TimestampUserdata::metamethod__eq(lua_State* L)
{
    TimestampUserdata* tsudRhs = checkPrivateUserdata<TimestampUserdata>(L, 1);
    TimestampUserdata* tsudLhs = checkPrivateUserdata<TimestampUserdata>(L, 2);
    
    bool result = tsudLhs->mTimestamp == tsudRhs->mTimestamp;
    lua_pushboolean(L, result);
    
    return 1;
}

int TimestampUserdata::metamethod__le(lua_State* L)
{
    TimestampUserdata* tsudRhs = checkPrivateUserdata<TimestampUserdata>(L, 1);
    TimestampUserdata* tsudLhs = checkPrivateUserdata<TimestampUserdata>(L, 2);
    
    bool result = tsudLhs->mTimestamp <= tsudRhs->mTimestamp;
    lua_pushboolean(L, result);
    
    return 1;
}

// userdata methods


/// Get time elapsed since time denoted by timestamp in microseconds.
// The value returned is stored within a dynamicany userdata as an Int64.
// @see dynamicany
// @return dynamicany userdata or nil. (error)
// @return error message.
// @function elapsed
int TimestampUserdata::elapsed(lua_State* L)
{
    int rv = 0;
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    
    Poco::Int64 elapsed = tsud->mTimestamp.elapsed();
    void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
    luaL_getmetatable(L, "Poco.DynamicAny.metatable");
    lua_setmetatable(L, -2);
    
    try
    {
        DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(elapsed);
        setPrivateUserdata(L, -1, daud);
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

/// Get the timestamp expressed in microseconds since the Unix epoch, midnight, January 1, 1970.
// The value returned is stored within a dynamicany userdata as an Int64.
// @see dynamicany
// @return dynamicany userdata or nil. (error)
// @return error message.
// @function epochMicroseconds
int TimestampUserdata::epochMicroseconds(lua_State* L)
{
    int rv = 0;
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    
    Poco::Int64 epochMs = tsud->mTimestamp.epochMicroseconds();
    
    void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
    luaL_getmetatable(L, "Poco.DynamicAny.metatable");
    lua_setmetatable(L, -2);
    
    try
    {
        DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(epochMs);
        setPrivateUserdata(L, -1, daud);
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

/// Gets the timestamp expressed in time_t (a Lua number). time_t base time is midnight, January 1, 1970. Resolution is one second.
// The value returned is stored within a dynamicany userdata as an Int64.
// @see dynamicany
// @return dynamicany userdata or nil. (error)
// @return error message.
// @function epochTime
int TimestampUserdata::epochTime(lua_State* L)
{
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    // Lua 5.1 and 5.2 represent os.time() values as lua_Numbers.
    lua_Number epoch = static_cast<lua_Number>(tsud->mTimestamp.epochTime());
    lua_pushnumber(L, epoch);
    
    return 1;
}

/// Gets the timestamp expressed in time_t (a Lua number). time_t base time is midnight, January 1, 1970. Resolution is one second.
// The value returned is stored within a dynamicany userdata as an Int64.
// @see dynamicany
// @return boolean
// @function isElapsed
int TimestampUserdata::isElapsed(lua_State* L)
{
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    
    Poco::Int64 interval = 0;
    
    if (lua_isnumber(L, 2))
        interval = luaL_checkinteger(L, 2);
    else
    {
        DynamicAnyUserdata* daud = checkPrivateUserdata<DynamicAnyUserdata>(L, 2);
        try
        {
            daud->mDynamicAny.convert(interval);
        }
        catch (const Poco::Exception& e)
        {
            pushPocoException(L, e);
            return lua_error(L);
        }
        catch (...)
        {
            pushUnknownException(L);
            return lua_error(L);
        }
    }
    
    int elapsed = tsud->mTimestamp.isElapsed(interval);
    lua_pushboolean(L, elapsed);
    
    return 1;
}

/// Gets the resolution in units per second. Since the timestamp has microsecond resolution, the returned value is always 1000000.
// @return number
// @function resolution
int TimestampUserdata::resolution(lua_State* L)
{
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    
    lua_Number res = tsud->mTimestamp.resolution();
    lua_pushnumber(L, res);
    
    return 1;
}

int TimestampUserdata::update(lua_State* L)
{
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    tsud->mTimestamp.update();
    
    return 0;
}

/// Gets the timestamp expressed in UTC-based time. UTC base time is midnight, October 15, 1582. Resolution is 100 nanoseconds.
// The value returned is stored within a dynamicany userdata as an Int64.
// @see dynamicany
// @return dynamicany userdata or nil. (error)
// @return error message.
// @function utcTime
int TimestampUserdata::utcTime(lua_State* L)
{
    int rv = 0;
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    
    Poco::DynamicAny val = tsud->mTimestamp.utcTime();
    
    void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
    luaL_getmetatable(L, "Poco.DynamicAny.metatable");
    lua_setmetatable(L, -2);
    
    try
    {
        DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(val);
        setPrivateUserdata(L, -1, daud);
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

} // LuaPoco
