/// base32encoder
// An ostream interface for outputting base32 encoded data.
// @module base32encoder
#include "Base32Encoder.h"
#include <Poco/Exception.h>
#include <cstring>

int luaopen_poco_base32encoder(lua_State* L)
{
    LuaPoco::Base32EncoderUserdata::registerBase32Encoder(L);
    return LuaPoco::loadConstructor(L, LuaPoco::Base32EncoderUserdata::Base32Encoder);
}

namespace LuaPoco
{

const char* POCO_BASE32ENCODER_METATABLE_NAME = "Poco.Base32Encoder.metatable";

Base32EncoderUserdata::Base32EncoderUserdata(std::ostream & ostream, bool padding, int ref)
    : mBase32Encoder(ostream, padding)
    , mUdReference(ref)
    , mClosed(false)
{
}

Base32EncoderUserdata::~Base32EncoderUserdata()
{
}

std::ostream& Base32EncoderUserdata::ostream()
{
    return mBase32Encoder;
}

// register metatable for this class
bool Base32EncoderUserdata::registerBase32Encoder(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "write", write },
        { "flush", flush },
        { "seek", seek },
        { "close", close },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_BASE32ENCODER_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new base32encoder userdata.
// @tparam userdata ostream destination for base 32 encoded data.
// @bool[opt] padding specifies to use padding or not.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see ostream
int Base32EncoderUserdata::Base32Encoder(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    OStream* os = checkPrivateUserdata<OStream>(L, firstArg);
    int options = 0;
    int padding = true;
    if (lua_isboolean(L, firstArg + 1)) { padding = lua_toboolean(L, firstArg + 1); }

    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    try
    {
        Base32EncoderUserdata* b32eud = new(lua_newuserdata(L, sizeof *b32eud))
            Base32EncoderUserdata(os->ostream(), padding, ref);
        setupPocoUserdata(L, b32eud, POCO_BASE32ENCODER_METATABLE_NAME);
        rv = 1;
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
    
    return rv;
}

// metamethod infrastructure
int Base32EncoderUserdata::metamethod__tostring(lua_State* L)
{
    Base32EncoderUserdata* b32eud = checkPrivateUserdata<Base32EncoderUserdata>(L, 1);
    lua_pushfstring(L, "Poco.Base32Encoder (%p)", static_cast<void*>(b32eud));
    
    return 1;
}

int Base32EncoderUserdata::metamethod__gc(lua_State* L)
{
    Base32EncoderUserdata* b32eud = checkPrivateUserdata<Base32EncoderUserdata>(L, 1);
    if (!b32eud->mClosed) { b32eud->mBase32Encoder.close(); }
    luaL_unref(L, LUA_REGISTRYINDEX, b32eud->mUdReference);
    b32eud->~Base32EncoderUserdata();

    return 0;
}
/// 
// @type base32encoder

/// Closes base32encoder
// Flushes all encoded output to the destination ostream.
// @function close
int Base32EncoderUserdata::close(lua_State* L)
{
    Base32EncoderUserdata* b32eud = checkPrivateUserdata<Base32EncoderUserdata>(L, 1);
    if (!b32eud->mClosed) { b32eud->mClosed = true; b32eud->mBase32Encoder.close(); }
    return 0;
}

} // LuaPoco
