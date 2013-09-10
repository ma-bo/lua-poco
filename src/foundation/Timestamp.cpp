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

UserdataType TimestampUserdata::getType()
{
	return Userdata_Timestamp;
}

bool TimestampUserdata::isCopyable()
{
	return true;
}

bool TimestampUserdata::copyToState(lua_State* L)
{
	void* ud = lua_newuserdata(L, sizeof *this);
	luaL_getmetatable(L, "Poco.Timestamp.metatable");
	lua_setmetatable(L, -2);
	TimestampUserdata* tsud = new(ud) TimestampUserdata(mTimestamp);
	
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

int TimestampUserdata::TimestampFromUtc(lua_State* L)
{
	int rv = 0;
	
	Poco::Int64 val = 0;
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	try
	{
		daud->mDynamicAny.convert(val);
		void *ud = lua_newuserdata(L, sizeof(TimestampUserdata));
		luaL_getmetatable(L, "Poco.Timestamp.metatable");
		lua_setmetatable(L, -2);
		TimestampUserdata* tsud = new (ud) TimestampUserdata(val);
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
int TimestampUserdata::Timestamp(lua_State* L)
{
	int rv = 0;
	int top = lua_gettop(L);
	
	if (top > 0)
	{
		Poco::Int64 val = 0;
		DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
			luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
		try
		{
			daud->mDynamicAny.convert(val);
			void *ud = lua_newuserdata(L, sizeof(TimestampUserdata));
			luaL_getmetatable(L, "Poco.Timestamp.metatable");
			lua_setmetatable(L, -2);
			TimestampUserdata* tsud = new (ud) TimestampUserdata(val);
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
	}
	else
	{
		void *ud = lua_newuserdata(L, sizeof(TimestampUserdata));
		luaL_getmetatable(L, "Poco.Timestamp.metatable");
		lua_setmetatable(L, -2);
		TimestampUserdata* tsud = new (ud) TimestampUserdata();
		rv = 1;
	}
	
	return rv;
}

	
// metamethod infrastructure
int TimestampUserdata::metamethod__gc(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	tsud->~TimestampUserdata();
	return 0;
}

int TimestampUserdata::metamethod__tostring(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
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
	
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, tsIndex, "Poco.Timestamp.metatable"));
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, otherIndex));
	
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
	
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, tsIndex, "Poco.Timestamp.metatable"));
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, otherIndex));
	
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
	TimestampUserdata* tsudLhs = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	TimestampUserdata* tsudRhs = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 2, "Poco.Timestamp.metatable"));
	
	bool result = tsudLhs->mTimestamp < tsudRhs->mTimestamp;
	lua_pushboolean(L, result);
	
	return 1;
}

int TimestampUserdata::metamethod__eq(lua_State* L)
{
	TimestampUserdata* tsudLhs = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	TimestampUserdata* tsudRhs = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 2, "Poco.Timestamp.metatable"));
	
	bool result = tsudLhs->mTimestamp == tsudRhs->mTimestamp;
	lua_pushboolean(L, result);
	
	return 1;
}

int TimestampUserdata::metamethod__le(lua_State* L)
{
	TimestampUserdata* tsudLhs = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	TimestampUserdata* tsudRhs = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 2, "Poco.Timestamp.metatable"));
	
	bool result = tsudLhs->mTimestamp <= tsudRhs->mTimestamp;
	lua_pushboolean(L, result);
	
	return 1;
}

// userdata methods
int TimestampUserdata::elapsed(lua_State* L)
{
	int rv = 0;
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	Poco::Int64 elapsed = tsud->mTimestamp.elapsed();
	void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
	luaL_getmetatable(L, "Poco.DynamicAny.metatable");
	lua_setmetatable(L, -2);
	
	try
	{
		DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(elapsed);
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

int TimestampUserdata::epochMicroseconds(lua_State* L)
{
	int rv = 0;
	
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	Poco::Int64 epochMs = tsud->mTimestamp.epochMicroseconds();
	
	void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
	luaL_getmetatable(L, "Poco.DynamicAny.metatable");
	lua_setmetatable(L, -2);
	
	try
	{
		DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(epochMs);
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

int TimestampUserdata::epochTime(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	// Lua 5.1 and 5.2 represent os.time() values as lua_Numbers.
	lua_Number epoch = static_cast<lua_Number>(tsud->mTimestamp.epochTime());
	lua_pushnumber(L, epoch);
	
	return 1;
}

int TimestampUserdata::isElapsed(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	Poco::Int64 interval = 0;
	
	if (lua_isnumber(L, 2))
		interval = luaL_checkinteger(L, 2);
	else
	{
		DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
			luaL_checkudata(L, 2, "Poco.DynamicAny.metatable"));
		try
		{
			daud->mDynamicAny.convert(interval);
		}
		catch (const Poco::Exception& e)
		{
			return pushPocoException(L, e);
		}
		catch (...)
		{
			return pushUnknownException(L);
		}
	}
	
	int elapsed = tsud->mTimestamp.isElapsed(interval);
	lua_pushboolean(L, elapsed);
	
	return 1;
}

int TimestampUserdata::resolution(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	lua_Number res = tsud->mTimestamp.resolution();
	lua_pushnumber(L, res);
	
	return 1;
}

int TimestampUserdata::update(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	tsud->mTimestamp.update();
	
	return 0;
}

int TimestampUserdata::utcTime(lua_State* L)
{
	int rv = 0;
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	Poco::DynamicAny val = tsud->mTimestamp.utcTime();
	
	void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
	luaL_getmetatable(L, "Poco.DynamicAny.metatable");
	lua_setmetatable(L, -2);
	
	try
	{
		DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(val);
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
