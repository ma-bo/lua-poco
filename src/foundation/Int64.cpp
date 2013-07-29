#include "Int64.h"

namespace LuaPoco
{

Int64Userdata::Int64Userdata(Poco::Int64 val)
{
}

Int64Userdata::Int64Userdata(Poco::UInt64 val)
{
}

Int64Userdata::~Int64Userdata()
{
}

UserdataType Int64Userdata::getType()
{
	return Userdata_Int64;
}
bool Int64Userdata::isCopyable()
{
	return true;
}

bool Int64Userdata::copyToState(lua_State* L)
{
	return false;
}

// register metatable for this class
bool Int64Userdata::registerInt64(lua_State* L)
{
	return false;
}
// Lua constructor
int Int64Userdata::Int64(lua_State* L)
{
	return 0;
}

// metamethod infrastructure
int Int64Userdata::metamethod__gc(lua_State* L)
{
	return 0;
}

int Int64Userdata::metamethod__tostring(lua_State* L)
{
	return 0;
}


int Int64Userdata::metamethod__add(lua_State* L)
{
	return 0;
}

int Int64Userdata::metamethod__sub(lua_State* L)
{
	return 0;
}

int Int64Userdata::metamethod__lt(lua_State* L)
{
	return 0;
}

int Int64Userdata::metamethod__le(lua_State* L)
{
	return 0;
}

int Int64Userdata::metamethod__gt(lua_State* L)
{
	return 0;
}

int Int64Userdata::metamethod__ge(lua_State* L)
{
	return 0;
}

} // LuaPoco
