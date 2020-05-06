/// Timestamps.
// Stores a monotonic* time value with (theoretical) microseconds resolution. Timestamps can be compared with each other and simple arithmetic is supported.
// Note: Timestamp values are only monotonic as long as the systems's clock is monotonic as well (and not, e.g. set back).
// Note: Timestamps are UTC (Coordinated Universal Time) based.
// @module timestamp

#include "Timestamp.h"
#include "Poco/Format.h"
#include "Poco/NumberFormatter.h"
#include <ctime>

int luaopen_poco_timestamp(lua_State* L)
{
    int rv = 0;
    
    if (LuaPoco::loadMetatables(L))
    {
        struct LuaPoco::CFunctions methods[] = 
        {
            { "new", LuaPoco::TimestampUserdata::Timestamp },
            { "__call", LuaPoco::TimestampUserdata::Timestamp },
            { "fromEpoch", LuaPoco::TimestampUserdata::TimestampFromEpoch },
            { "fromUTC", LuaPoco::TimestampUserdata::TimestampFromUtc },
            { NULL, NULL}
        };
        
        lua_createtable(L, 0, 1);
        setCFunctions(L, methods);
        // lua_pushvalue(L, -1);
        // lua_setfield(L, -2, "__index");
        lua_pushvalue(L, -1);
        lua_setmetatable(L, -2);
        
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

const char* POCO_TIMESTAMP_METATABLE_NAME = "Poco.Timestamp.metatable";

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
    TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud)) TimestampUserdata(mTimestamp);
    setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool TimestampUserdata::registerTimestamp(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "__add", metamethod__add },
        { "__sub", metamethod__sub },
        { "__eq", metamethod__eq },
        { "__lt", metamethod__lt },
        { "__le", metamethod__le },
        { "elapsed", elapsed },
        { "epochMicroseconds", epochMicroseconds },
        { "epochTime", epochTime },
        { "isElapsed", isElapsed },
        { "update", update },
        { "utcTime", utcTime },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_TIMESTAMP_METATABLE_NAME, methods);
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
        TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud)) 
            TimestampUserdata(Poco::Timestamp::fromEpochTime(val));
        
        setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
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

/// Constructs a new timestamp userdata from 64-bit UTC value encoded as a string.
// Creates a timestamp from a UTC time value (100 nanosecond intervals since midnight, October 15, 1582).
// @string value used to create timestamp.
// @return userdata or nil. (error)
// @return error message.
// @function fromUTC
int TimestampUserdata::TimestampFromUtc(lua_State* L)
{
    int rv = 0;
    Poco::Int64 val = static_cast<Poco::Int64>(luaL_checkinteger(L, 1));
    
    try
    {
        TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud)) TimestampUserdata(val);
        setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
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
    
    try
    {
        TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud)) TimestampUserdata();
        setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
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
    lua_pushfstring(L, "Poco.Timestamp (%p)", static_cast<void*>(tsud));
    return 1;
}

int TimestampUserdata::metamethod__add(lua_State* L)
{
    int rv = 0;
    int tsIndex = 1;
    int stringIndex = 2;

    if (lua_isstring(L, 1)) { stringIndex = 1; }
    else if (lua_isstring(L, 2)) { stringIndex = 2; };

    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, tsIndex);
    const char* toAddStr = luaL_checkstring(L, stringIndex);
    
    try
    {
        Poco::Int64 val;
        Poco::strToInt(toAddStr, val, 10);
        Poco::Timestamp newTs= tsud->mTimestamp + val;
        TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud)) TimestampUserdata(newTs);
        setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
        
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
    int stringIndex = 0;

    if (lua_isstring(L, 1)) { tsIndex = 2; stringIndex = 1; }
    else if (lua_isstring(L, 2)) { tsIndex = 1; stringIndex = 2; };
    
    
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, tsIndex);
    
    try
    {
        Poco::Timestamp newTs(0);
        
        if (stringIndex)
        {
            const char* toSubStr = luaL_checkstring(L, otherIndex);
            Poco::Timestamp::TimeDiff val = 0;
            Poco::strToInt(toSubStr, val, 10);
            newTs = tsud->mTimestamp - val;
        }
        else
        {
            TimestampUserdata* tsudOther = checkPrivateUserdata<TimestampUserdata>(L, otherIndex);
            newTs = tsud->mTimestamp - tsudOther->mTimestamp;
        }

        TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud)) TimestampUserdata(newTs);
        setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
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
// The value returned is returned as an Int64 encoded in a string.
// @return string or nil. (error)
// @return error message.
// @function elapsed
int TimestampUserdata::elapsed(lua_State* L)
{
    int rv = 0;
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    
    Poco::Int64 elapsed = tsud->mTimestamp.elapsed();
    
    try
    {
        std::string elapsedStr = Poco::NumberFormatter::format(elapsed);
        lua_pushlstring(L, elapsedStr.c_str(), elapsedStr.size());
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
// The value returned as an Int64 encoded in a string
// @return string userdata or nil. (error)
// @return error message.
// @function epochMicroseconds
int TimestampUserdata::epochMicroseconds(lua_State* L)
{
    int rv = 0;
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    
    Poco::Int64 epochMs = tsud->mTimestamp.epochMicroseconds();
    
    try
    {
        std::string epochStr = Poco::NumberFormatter::format(epochMs);
        lua_pushlstring(L, epochStr.c_str(), epochStr.size());
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

/// Gets the timestamp expressed in time_t (a Lua integer). time_t base time is midnight, January 1, 1970. Resolution is one second.
// @return int
// @return error message.
// @function epochTime
int TimestampUserdata::epochTime(lua_State* L)
{
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    lua_pushinteger(L, tsud->mTimestamp.epochTime());
    
    return 1;
}

/// Returns true if and only if the given interval (in microseconds) has passed since the time denoted by the timestamp.
// Parameter can be passed as an integer for numbers not requiring 64-bits to represent, otherwise passed as a 64-bit number encoded as a string.
// @param (integer or string) interval time duration in microseconds to check has elapsed.
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
        const char* intervalStr = luaL_checkstring(L, 1);
        
        try
        {
            Poco::strToInt(intervalStr, interval, 10);
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
    
    lua_pushinteger(L, tsud->mTimestamp.resolution());
    
    return 1;
}

/// Updates the existing timestamp with a the current timestamp value.
// @function update
int TimestampUserdata::update(lua_State* L)
{
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    tsud->mTimestamp.update();
    
    return 0;
}

/// Gets the timestamp expressed in UTC-based time. UTC base time is midnight, October 15, 1582.
// Resolution is 100 nanoseconds.  The value returned is an Int64 encoded as a string.
// @return string or nil. (error)
// @return error message.
// @function utcTime
int TimestampUserdata::utcTime(lua_State* L)
{
    int rv = 0;
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 1);
    
    Poco::Int64 val = tsud->mTimestamp.utcTime();
    
    try
    {
        std::string utcStr = Poco::NumberFormatter::format(tsud->mTimestamp.utcTime());
        lua_pushlstring(L, utcStr.c_str(), utcStr.size());
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
