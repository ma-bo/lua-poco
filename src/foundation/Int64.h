#ifndef LUA_POCO_INT64_H
#define LUA_POCO_INT64_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/Types.h"

namespace LuaPoco
{

class Int64Userdata
{
public:
	Int64Userdata(Poco::Int64 val);
	Int64Userdata(Poco::UInt64 val);
	virtual ~Int64Userdata();
	virtual UserdataType getType();
	virtual bool isCopyable();
	virtual bool copyToState(lua_State* L);
	
	// register metatable for this class
	static bool registerInt64(lua_State* L);
private:
	// Lua constructor
	static int Int64(lua_State* L);
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	static int metamethod__tostring(lua_State* L);
	static int metamethod__add(lua_State* L);
	static int metamethod__sub(lua_State* L);
	static int metamethod__lt(lua_State* L);
	static int metamethod__le(lua_State* L);
	static int metamethod__gt(lua_State* L);
	static int metamethod__ge(lua_State* L);
	
	Poco::UInt64 mUInt64;
	Poco::Int64 mInt64;
};

} // LuaPoco


#endif
