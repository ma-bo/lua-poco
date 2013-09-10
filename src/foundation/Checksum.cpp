#include "Checksum.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_checksum(lua_State* L)
{
	return LuaPoco::loadConstructor(L, LuaPoco::ChecksumUserdata::Checksum);
}

namespace LuaPoco
{

ChecksumUserdata::ChecksumUserdata(Poco::Checksum::Type t) :
	mChecksum(t)
{
}

ChecksumUserdata::~ChecksumUserdata()
{
}

UserdataType ChecksumUserdata::getType()
{
	return Userdata_Checksum;
}

bool ChecksumUserdata::isCopyable()
{
	return true;
}

bool ChecksumUserdata::copyToState(lua_State *L)
{
	void* ud = lua_newuserdata(L, sizeof(ChecksumUserdata));
	luaL_getmetatable(L, "Poco.Checksum.metatable");
	lua_setmetatable(L, -2);
	
	ChecksumUserdata* csud = new(ud) ChecksumUserdata(csud->mChecksum.type());
	csud->mChecksum = mChecksum;
	return true;
}

// register metatable for this class
bool ChecksumUserdata::registerChecksum(lua_State* L)
{
	luaL_newmetatable(L, "Poco.Checksum.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushstring(L, "Poco.Checksum.metatable");
	lua_setfield(L, -2, "poco.userdata");
	
	// methods
	lua_pushcfunction(L, update);
	lua_setfield(L, -2, "update");
	lua_pushcfunction(L, checksum);
	lua_setfield(L, -2, "checksum");
	lua_pushcfunction(L, type);
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);
	return true;
}

int ChecksumUserdata::Checksum(lua_State* L)
{
	int top = lua_gettop(L);
	Poco::Checksum::Type type = Poco::Checksum::TYPE_ADLER32;
	const char* typeStr;
	if (top > 0)
	{
		typeStr = luaL_checkstring(L, 1);
		if (std::strcmp(typeStr, "CRC32") == 0)
			type = Poco::Checksum::TYPE_CRC32;
	}
	
	void* ud = lua_newuserdata(L, sizeof(ChecksumUserdata));
	luaL_getmetatable(L, "Poco.Checksum.metatable");
	lua_setmetatable(L, -2);
	
	ChecksumUserdata* csud = new(ud) ChecksumUserdata(type);
	return 1;
}

// metamethod infrastructure
int ChecksumUserdata::metamethod__gc(lua_State* L)
{
	ChecksumUserdata* csud = reinterpret_cast<ChecksumUserdata*>(
		luaL_checkudata(L, 1, "Poco.Checksum.metatable"));
	csud->~ChecksumUserdata();
	
	return 0;
}

int ChecksumUserdata::metamethod__tostring(lua_State* L)
{
	ChecksumUserdata* csud = reinterpret_cast<ChecksumUserdata*>(
		luaL_checkudata(L, 1, "Poco.Checksum.metatable"));
	
	lua_pushfstring(L, "Poco.Checksum (%p)", reinterpret_cast<void*>(csud));
	return 1;
}

// userdata methods
int ChecksumUserdata::update(lua_State* L)
{
	ChecksumUserdata* csud = reinterpret_cast<ChecksumUserdata*>(
		luaL_checkudata(L, 1, "Poco.Checksum.metatable"));
	
	luaL_checkany(L, 2);
	if (lua_isnumber(L, 2))
	{
		char val = static_cast<char>(lua_tointeger(L, 2));
		csud->mChecksum.update(val);
	}
	else if (lua_isstring(L, 2))
	{
		size_t strSize;
		const char* str = lua_tolstring(L, 2, &strSize);
		csud->mChecksum.update(str, strSize);
	}
	else
		luaL_error(L, "invalid type %s, update requires number (byte) or string", luaL_typename(L, 2));
	
	return 0;
}

int ChecksumUserdata::checksum(lua_State* L)
{
	ChecksumUserdata* csud = reinterpret_cast<ChecksumUserdata*>(
		luaL_checkudata(L, 1, "Poco.Checksum.metatable"));
	
	lua_pushinteger(L, csud->mChecksum.checksum());
	return 1;
}

int ChecksumUserdata::type(lua_State* L)
{
	ChecksumUserdata* csud = reinterpret_cast<ChecksumUserdata*>(
		luaL_checkudata(L, 1, "Poco.Checksum.metatable"));
	
	const char* checksumType = "ADLER32";
	if (csud->mChecksum.type() == Poco::Checksum::TYPE_CRC32)
		checksumType = "CRC32";
	
	lua_pushstring(L, checksumType);
	return 1;
}

} // LuaPoco
