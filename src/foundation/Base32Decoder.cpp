/// base32decoder
// An istream interface for inputting base32 encoded data.
// @module base32decoder
#include "Base32Decoder.h"
#include <Poco/Base32Encoder.h>
#include <Poco/Exception.h>
#include <cstring>

int luaopen_poco_base32decoder(lua_State* L)
{
    LuaPoco::Base32DecoderUserdata::registerBase32Decoder(L);
    return LuaPoco::loadConstructor(L, LuaPoco::Base32DecoderUserdata::Base32Decoder);
}

namespace LuaPoco
{

const char* POCO_BASE32DECODER_METATABLE_NAME = "Poco.Base32Decoder.metatable";

Base32DecoderUserdata::Base32DecoderUserdata(std::istream & istream, int ref)
    : mBase32Decoder(istream)
    , mUdReference(ref)
    , mClosed(false)
{
}

Base32DecoderUserdata::~Base32DecoderUserdata()
{
}

std::istream& Base32DecoderUserdata::istream()
{
    return mBase32Decoder;
}

// register metatable for this class
bool Base32DecoderUserdata::registerBase32Decoder(lua_State* L)
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
    
    setupUserdataMetatable(L, POCO_BASE32DECODER_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new base32decoder userdata.
// @tparam userdata istream destination for base32 encoded data.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see istream
int Base32DecoderUserdata::Base32Decoder(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    IStream* is = checkPrivateUserdata<IStream>(L, firstArg);

    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    try
    {
        Base32DecoderUserdata* b32dud = new(lua_newuserdata(L, sizeof *b32dud))
            Base32DecoderUserdata(is->istream(), ref);
        setupPocoUserdata(L, b32dud, POCO_BASE32DECODER_METATABLE_NAME);
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
int Base32DecoderUserdata::metamethod__tostring(lua_State* L)
{
    Base32DecoderUserdata* b32dud = checkPrivateUserdata<Base32DecoderUserdata>(L, 1);
    lua_pushfstring(L, "Poco.Base32Decoder (%p)", static_cast<void*>(b32dud));
    
    return 1;
}

int Base32DecoderUserdata::metamethod__gc(lua_State* L)
{
    Base32DecoderUserdata* b32dud = checkPrivateUserdata<Base32DecoderUserdata>(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, b32dud->mUdReference);
    b32dud->~Base32DecoderUserdata();

    return 0;
}
/// 
// @type base32decoder

} // LuaPoco
