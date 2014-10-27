/// CRC-32 or Adler-32 checksums.
// A userdata object that generates CRC-32 or Adler-32 checksums.
// Input can be either a string or an individual character represented as a number.
//
// Note: checksum userdata is copyable between poco threads.
// @module checksum

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

bool ChecksumUserdata::copyToState(lua_State *L)
{
    void* ud = lua_newuserdata(L, sizeof(ChecksumUserdata));
    luaL_getmetatable(L, "Poco.Checksum.metatable");
    lua_setmetatable(L, -2);
    
    ChecksumUserdata* csud = new(ud) ChecksumUserdata(csud->mChecksum.type());
    setPrivateUserdata(L, -1, csud);
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
/// construct a new checksum userdata using CRC-32 or Adler-32 (default) algorithms.
// @string[opt] type "CRC32" or "ADLER32" (default, if not specified)
// @return userdata or nil. (error)
// @return error message
// @function new
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
    setPrivateUserdata(L, -1, csud);
    
    return 1;
}

// metamethod infrastructure

///
// @type checksum
int ChecksumUserdata::metamethod__tostring(lua_State* L)
{
    ChecksumUserdata* csud = checkPrivateUserdata<ChecksumUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.Checksum (%p)", static_cast<void*>(csud));
    return 1;
}

// userdata methods

/// updates the checksum with the data passed.
// @string data a string containing the data or a Lua number to be cast to a char.
// @function update
int ChecksumUserdata::update(lua_State* L)
{
    ChecksumUserdata* csud = checkPrivateUserdata<ChecksumUserdata>(L, 1);
    
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

/// returns the current checksum value.
// @return value as a number.
// @function checksum
int ChecksumUserdata::checksum(lua_State* L)
{
    ChecksumUserdata* csud = checkPrivateUserdata<ChecksumUserdata>(L, 1);
    
    lua_pushinteger(L, csud->mChecksum.checksum());
    return 1;
}

/// returns the checksum algorithm in use.
// @return algorithm name as a string.
// @function type
int ChecksumUserdata::type(lua_State* L)
{
    ChecksumUserdata* csud = checkPrivateUserdata<ChecksumUserdata>(L, 1);
    
    const char* checksumType = "ADLER32";
    if (csud->mChecksum.type() == Poco::Checksum::TYPE_CRC32)
        checksumType = "CRC32";
    
    lua_pushstring(L, checksumType);
    return 1;
}

} // LuaPoco
