#include "DynamicAny.h"

namespace LuaPoco
{

DynamicAnyUserdata::DynamicAnyUserdata(const Poco::DynamicAny& da) : mDynamicAny(da)
{
}

DynamicAnyUserdata::~DynamicAnyUserdata()
{
}

UserdataType DynamicAnyUserdata::getType()
{
	return Userdata_DynamicAny;
}

bool DynamicAnyUserdata::isCopyable()
{
	return true;
}

bool DynamicAnyUserdata::copyToState(lua_State* L)
{
	void* ud = lua_newuserdata(L, sizeof *this);
	luaL_getmetatable(L, "Poco.DynamicAny.metatable");
	lua_setmetatable(L, -2);
	
	DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(mDynamicAny);
	
	return true;
}

// register metatable for this class
bool DynamicAnyUserdata::registerDynamicAny(lua_State* L)
{
	bool result = false;
	if (!lua_istable(L, -1))
		return result;
	
	// constructor
	lua_pushcfunction(L, DynamicAny);
	lua_setfield(L, -2, "DynamicAny");
	
	luaL_newmetatable(L, "Poco.DynamicAny.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	// methods
	lua_pushcfunction(L, isNumeric);
	lua_setfield(L, -2, "isNumeric");
	lua_pushcfunction(L, isInteger);
	lua_setfield(L, -2, "isInteger");
	lua_pushcfunction(L, isSigned);
	lua_setfield(L, -2, "isSigned");
	lua_pushcfunction(L, isString);
	lua_setfield(L, -2, "isString");
	lua_pushcfunction(L, toNumber);
	lua_setfield(L, -2, "toNumber");
	lua_pushcfunction(L, toNumber);
	lua_setfield(L, -2, "toString");
	lua_pushcfunction(L, toBoolean);
	lua_setfield(L, -2, "toBoolean");
	lua_pushcfunction(L, metamethod__add);
	lua_setfield(L, -2, "__add");
	lua_pop(L, 1);
	result = true;
	
	return result;
}

// Lua constructor
int DynamicAnyUserdata::DynamicAny(lua_State* L)
{
	int rv = 0;
	
	luaL_checkany(L, 1);
	int type = lua_type(L, 1);
	try
	{
		if (type == LUA_TNUMBER)
		{
			void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
			luaL_getmetatable(L, "Poco.DynamicAny.metatable");
			lua_setmetatable(L, -2);
			lua_Number val = lua_tonumber(L, 1);
			DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(val);
			rv = 1;
		}
		else if (type == LUA_TSTRING)
		{
			void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
			luaL_getmetatable(L, "Poco.DynamicAny.metatable");
			lua_setmetatable(L, -2);
			const char* val = lua_tostring(L, 1);
			DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(val);
			rv = 1;
		}
		else if (type == LUA_TBOOLEAN)
		{
			void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
			luaL_getmetatable(L, "Poco.DynamicAny.metatable");
			lua_setmetatable(L, -2);
			bool val = lua_toboolean(L, 1);
			DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(val);
			rv = 1;
		}
		else if (type == LUA_TUSERDATA)
		{
			void *udFrom = luaL_checkudata(L, 1, "Poco.DynamicAny.metatable");
			void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
			luaL_getmetatable(L, "Poco.DynamicAny.metatable");
			lua_setmetatable(L, -2);
			DynamicAnyUserdata* daudFrom = reinterpret_cast<DynamicAnyUserdata*>(udFrom);
			DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(*daudFrom);
			rv = 1;
		}
		else
		{
			lua_pushnil(L);
			lua_pushstring(L,"DynamicAny requires a number, string, boolean, or a Poco.DynamicAny");
			rv = 2;
		}
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

// metamethod infrastructure
int DynamicAnyUserdata::metamethod__gc(lua_State* L)
{
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	daud->~DynamicAnyUserdata();
	
	return 0;
}

int DynamicAnyUserdata::metamethod__tostring(lua_State* L)
{
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	
	lua_pushfstring(L, "Poco.DynamicAny (%p)", reinterpret_cast<void*>(daud));
	return 1;
}

// methods
int DynamicAnyUserdata::isNumeric(lua_State* L)
{
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	
	int isnumeric = daud->mDynamicAny.isNumeric();
	lua_pushboolean(L, isnumeric);
	
	return 1;
}

int DynamicAnyUserdata::isInteger(lua_State* L)
{
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	
	int isinteger = daud->mDynamicAny.isInteger();
	lua_pushboolean(L, isinteger);
	
	return 1;
}

int DynamicAnyUserdata::isSigned(lua_State* L)
{
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	
	int issigned = daud->mDynamicAny.isSigned();
	lua_pushboolean(L, issigned);
	
	return 1;
}

int DynamicAnyUserdata::isString(lua_State* L)
{
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	
	int isstring = daud->mDynamicAny.isString();
	lua_pushboolean(L, isstring);
	
	return 1;
}

int DynamicAnyUserdata::toNumber(lua_State* L)
{
	int rv = 0;
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	try
	{
		lua_Number result;
		daud->mDynamicAny.convert(result);
		lua_pushnumber(L, result);
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

int DynamicAnyUserdata::toString(lua_State* L)
{
	int rv = 0;
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	
	try
	{
		std::string result;
		daud->mDynamicAny.convert(result);
		lua_pushlstring(L, result.c_str(), result.size());
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

int DynamicAnyUserdata::toBoolean(lua_State* L)
{
	int rv = 0;
	DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
		luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
	
	try
	{
		bool result;
		daud->mDynamicAny.convert(result);
		lua_pushboolean(L, result);
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

int DynamicAnyUserdata::metamethod__add(lua_State* L)
{
	int rv = 0;
	int udIndex = 1;
	int valIndex = 2;
	
	DynamicAnyUserdata* daud;
	lua_Number val;
	
	if (!lua_isuserdata(L, 1))
	{
		udIndex = 2;
		valIndex = 1;
	}
	
	daud = reinterpret_cast<DynamicAnyUserdata*>(luaL_checkudata(L, udIndex, "Poco.DynamicAny.metatable"));
	val = luaL_checknumber(L, valIndex);
	
	try
	{
		val = daud->mDynamicAny + val;
		lua_pushnumber(L, val);
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
