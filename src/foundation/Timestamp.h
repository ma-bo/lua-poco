#ifndef LUA_POCO_TIMESTAMP_H
#define LUA_POCO_TIMESTAMP_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "Poco/Timestamp.h"

extern "C"
{
int luaopen_poco_timestamp(lua_State* L);
}

namespace LuaPoco
{

class TimestampUserdata : public Userdata
{
public:
	TimestampUserdata();
	TimestampUserdata(Poco::Timestamp::TimeVal tv);
	TimestampUserdata(const Poco::Timestamp& ts);
	virtual ~TimestampUserdata();
	virtual UserdataType getType();
	virtual bool isCopyable();
	virtual bool copyToState(lua_State* L);
	Poco::Timestamp mTimestamp;
	
	// register metatable for this class
	static bool registerTimestamp(lua_State* L);
	// standalone functions that create a TimestampUserdata
	static int TimestampFromEpoch(lua_State* L);
	static int TimestampFromUtc(lua_State* L);
		// Lua constructor
	static int Timestamp(lua_State* L);
private:
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	//static int metamethod__index(lua_State* L);
	//static int metamethod__newindex(lua_State* L);
	static int metamethod__tostring(lua_State* L);
	static int metamethod__add(lua_State* L);
	static int metamethod__sub(lua_State* L);
	static int metamethod__lt(lua_State* L);
	static int metamethod__le(lua_State* L);
	static int metamethod__eq(lua_State* L);
	
	// userdata methods
	static int elapsed(lua_State* L);
	static int epochMicroseconds(lua_State* L);
	static int epochTime(lua_State* L);
	static int isElapsed(lua_State* L);
	static int resolution(lua_State* L);
	static int update(lua_State* L);
	static int utcTime(lua_State* L);
};

} // LuaPoco

#endif
