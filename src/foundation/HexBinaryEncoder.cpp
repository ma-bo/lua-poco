/// hexbinaryencoder
// An ostream interface for outputting base32 encoded data.
// @module hexbinaryencoder
#include "HexBinaryEncoder.h"
#include <Poco/Exception.h>

int luaopen_poco_hexbinaryencoder(lua_State* L)
{
    LuaPoco::HexBinaryEncoderUserdata::registerHexBinaryEncoder(L);
    return LuaPoco::loadConstructor(L, LuaPoco::HexBinaryEncoderUserdata::HexBinaryEncoder);
}

namespace LuaPoco
{

const char* POCO_HEXBINARYENCODER_METATABLE_NAME = "Poco.HexBinaryEncoder.metatable";

HexBinaryEncoderUserdata::HexBinaryEncoderUserdata(std::ostream & ostream, int ref)
    : mHexBinaryEncoder(ostream)
    , mUdReference(ref)
{
}

HexBinaryEncoderUserdata::~HexBinaryEncoderUserdata()
{
}

std::ostream& HexBinaryEncoderUserdata::ostream()
{
    return mHexBinaryEncoder;
}

// register metatable for this class
bool HexBinaryEncoderUserdata::registerHexBinaryEncoder(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "write", write },
        { "flush", flush },
        { "seek", seek },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_HEXBINARYENCODER_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new hexbinaryencoder userdata.
// @tparam userdata ostream destination for base 32 encoded data.
// @bool[opt] padding specifies to use padding or not.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see ostream
int HexBinaryEncoderUserdata::HexBinaryEncoder(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    OStream* os = checkPrivateUserdata<OStream>(L, firstArg);

    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    HexBinaryEncoderUserdata* hbeud = NULL;
    void* p = lua_newuserdata(L, sizeof *hbeud);
    
    try
    {
        hbeud = new(p) HexBinaryEncoderUserdata(os->ostream(), ref);
    }
    catch (const std::exception& e)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        return pushException(L, e);
    }
    
    setupPocoUserdata(L, hbeud, POCO_HEXBINARYENCODER_METATABLE_NAME);
    return 1;
}

// metamethod infrastructure
int HexBinaryEncoderUserdata::metamethod__tostring(lua_State* L)
{
    HexBinaryEncoderUserdata* hbeud = checkPrivateUserdata<HexBinaryEncoderUserdata>(L, 1);
    lua_pushfstring(L, "Poco.HexBinaryEncoder (%p)", static_cast<void*>(hbeud));
    
    return 1;
}

int HexBinaryEncoderUserdata::metamethod__gc(lua_State* L)
{
    HexBinaryEncoderUserdata* hbeud = checkPrivateUserdata<HexBinaryEncoderUserdata>(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, hbeud->mUdReference);
    hbeud->~HexBinaryEncoderUserdata();

    return 0;
}
/// 
// @type hexbinaryencoder

} // LuaPoco

