/// hexbinarydecoder
// An istream interface for inputting hexbinary encoded data.
// @module hexbinarydecoder
#include "HexBinaryDecoder.h"
#include <Poco/HexBinaryEncoder.h>
#include <Poco/Exception.h>

int luaopen_poco_hexbinarydecoder(lua_State* L)
{
    LuaPoco::HexBinaryDecoderUserdata::registerHexBinaryDecoder(L);
    return LuaPoco::loadConstructor(L, LuaPoco::HexBinaryDecoderUserdata::HexBinaryDecoder);
}

namespace LuaPoco
{

const char* POCO_HEXBINARYDECODER_METATABLE_NAME = "Poco.HexBinaryDecoder.metatable";

HexBinaryDecoderUserdata::HexBinaryDecoderUserdata(std::istream & istream, int ref)
    : mHexBinaryDecoder(istream)
    , mUdReference(ref)
{
}

HexBinaryDecoderUserdata::~HexBinaryDecoderUserdata()
{
}

std::istream& HexBinaryDecoderUserdata::istream()
{
    return mHexBinaryDecoder;
}

// register metatable for this class
bool HexBinaryDecoderUserdata::registerHexBinaryDecoder(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "read", read },
        { "lines", lines },
        { "seek", seek },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_HEXBINARYDECODER_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new hexbinarydecoder userdata.
// @tparam userdata istream destination for hexbinary encoded data.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see istream
int HexBinaryDecoderUserdata::HexBinaryDecoder(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    IStream* is = checkPrivateUserdata<IStream>(L, firstArg);

    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    try
    {
        HexBinaryDecoderUserdata* hbdud = new(lua_newuserdata(L, sizeof *hbdud))
            HexBinaryDecoderUserdata(is->istream(), ref);
        setupPocoUserdata(L, hbdud, POCO_HEXBINARYDECODER_METATABLE_NAME);
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

// metamethod infrastructure
int HexBinaryDecoderUserdata::metamethod__tostring(lua_State* L)
{
    HexBinaryDecoderUserdata* hbdud = checkPrivateUserdata<HexBinaryDecoderUserdata>(L, 1);
    lua_pushfstring(L, "Poco.HexBinaryDecoder (%p)", static_cast<void*>(hbdud));
    
    return 1;
}

int HexBinaryDecoderUserdata::metamethod__gc(lua_State* L)
{
    HexBinaryDecoderUserdata* hbdud = checkPrivateUserdata<HexBinaryDecoderUserdata>(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, hbdud->mUdReference);
    hbdud->~HexBinaryDecoderUserdata();

    return 0;
}
/// 
// @type hexbinarydecoder

} // LuaPoco
