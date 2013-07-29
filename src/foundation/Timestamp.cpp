#include "Timestamp.h"

namespace LuaPoco
{

TimestampUserdata::TimestampUserdata(Poco::Timestamp::TimeVal tv)
{
	mTimestamp = new Poco::Timestamp(tv);
}

TimestampUserdata::TimestampUserdata(const Poco::Timestamp& ts)
{
	mTimestamp = new Poco::Timestamp(ts);
}

TimestampUserdata::~TimestampUserdata()
{
	delete mTimestamp;
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
	TimestampUserdata* tsud = new(ud) TimestampUserdata(*mTimestamp);
	
	return true;
}

// register metatable for this class
bool TimestampUserdata::registerTimestamp(lua_State* L)
{
	bool result = false;
	if (!lua_istable(L, -1))
		return result;
	
	// constructor: poco.Timestamp()
	lua_pushcfunction(L, Timestamp);
	lua_setfield(L, -2, "Timestamp");
	
	luaL_newmetatable(L, "Poco.Timestamp.metatable");
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
	lua_pop(L, 1);
	
	return result;
}

// standalone functions that create a TimestampUserdata
int TimestampUserdata::TimestampFromEpoch(lua_State* L)
{
	return 0;
}

int TimestampUserdata::TimestampFromUtc(lua_State* L)
{
	return 0;
}

// Lua constructor
int TimestampUserdata::Timestamp(lua_State* L)
{

	
	return 0;
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
	
	return 0;
}

int TimestampUserdata::metamethod__sub(lua_State* L)
{
	return 0;
}

int TimestampUserdata::metamethod__lt(lua_State* L)
{
	return 0;
}

int TimestampUserdata::metamethod__le(lua_State* L)
{
	return 0;
}

int TimestampUserdata::metamethod__gt(lua_State* L)
{
	return 0;
}

int TimestampUserdata::metamethod__ge(lua_State* L)
{
	return 0;
}
	
// userdata methods
int TimestampUserdata::elapsed(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	Poco::Int64 elapsed = tsud->mTimestamp->elapsed();
	// TODO: return something that wraps an Int64
	
	return 0;
}

int TimestampUserdata::epochMicroseconds(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	Poco::Int64 epochMs = tsud->mTimestamp->epochMicroseconds();
	// TODO: return something that wraps an Int64
	
	return 0;
}

int TimestampUserdata::epochTime(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	// Lua 5.1 and 5.2 represent os.time() values as lua_Numbers.
	lua_Number epoch = static_cast<lua_Number>(tsud->mTimestamp->epochTime());
	lua_pushnumber(L, epoch);
	
	return 1;
}

int TimestampUserdata::isElapsed(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	// TODO: get an Int64 from lua stack and put it in interval.
	Poco::Int64 interval = 0;
	
	int elapsed = tsud->mTimestamp->isElapsed(interval);
	lua_pushboolean(L, elapsed);
	
	return 1;
}

int TimestampUserdata::resolution(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	lua_Number res = tsud->mTimestamp->resolution();
	lua_pushnumber(L, res);
	
	return 1;
}

int TimestampUserdata::update(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	tsud->mTimestamp->update();
	
	return 0;
}

int TimestampUserdata::utcTime(lua_State* L)
{
	TimestampUserdata* tsud = reinterpret_cast<TimestampUserdata*>(
		luaL_checkudata(L, 1, "Poco.Timestamp.metatable"));
	
	return 0;
}

} // LuaPoco
