/// base64encoder
// An ostream interface for outputting base64 encoded data.
// @module base64encoder
#include "Base64Encoder.h"
#include <Poco/Exception.h>
#include <cstring>

int luaopen_poco_base64encoder(lua_State* L)
{
    LuaPoco::Base64EncoderUserdata::registerBase64Encoder(L);
    return LuaPoco::loadConstructor(L, LuaPoco::Base64EncoderUserdata::Base64Encoder);
}

namespace LuaPoco
{

const char* POCO_BASE64ENCODER_METATABLE_NAME = "Poco.Base64Encoder.metatable";

Base64EncoderUserdata::Base64EncoderUserdata(std::ostream & ostream, int options, int ref)
    : mBase64Encoder(ostream, options)
    , mUdReference(ref)
    , mClosed(false)
{
}

Base64EncoderUserdata::~Base64EncoderUserdata()
{
}

std::ostream& Base64EncoderUserdata::ostream()
{
    return mBase64Encoder;
}

// register metatable for this class
bool Base64EncoderUserdata::registerBase64Encoder(lua_State* L)
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
    
    setupUserdataMetatable(L, POCO_BASE64ENCODER_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new base64encoder userdata.
// @tparam userdata ostream destination for base 64 encoded data.
// @string[opt] options "url_encoding", "no_padding", or both separated by space or commas.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see ostream
int Base64EncoderUserdata::Base64Encoder(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    OStream* os = checkPrivateUserdata<OStream>(L, firstArg);
    int options = 0;
    const char* optionsStr = lua_tostring(L, firstArg + 1);
    if (optionsStr)
    {
        if (std::strstr(optionsStr, "url_encoding")) { options |= Poco::BASE64_URL_ENCODING; }
        if (std::strstr(optionsStr, "no_padding")) { options |= Poco::BASE64_NO_PADDING; }
    }

    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    Base64EncoderUserdata* b64eud = NULL;
    void* p = lua_newuserdata(L, sizeof *b64eud);
    
    try
    {
        b64eud = new(p) Base64EncoderUserdata(os->ostream(), options, ref);
    }
    catch (const std::exception& e)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        return pushException(L, e);
    }
    
    setupPocoUserdata(L, b64eud, POCO_BASE64ENCODER_METATABLE_NAME);
    return 1;
}

// metamethod infrastructure
int Base64EncoderUserdata::metamethod__tostring(lua_State* L)
{
    Base64EncoderUserdata* b64eud = checkPrivateUserdata<Base64EncoderUserdata>(L, 1);
    lua_pushfstring(L, "Poco.Base64Encoder (%p)", static_cast<void*>(b64eud));
    
    return 1;
}

int Base64EncoderUserdata::metamethod__gc(lua_State* L)
{
    Base64EncoderUserdata* b64eud = checkPrivateUserdata<Base64EncoderUserdata>(L, 1);
    if (!b64eud->mClosed) { b64eud->mBase64Encoder.close(); }
    luaL_unref(L, LUA_REGISTRYINDEX, b64eud->mUdReference);
    b64eud->~Base64EncoderUserdata();

    return 0;
}
/// 
// @type base64encoder

/// Closes base64encoder
// Flushes all encoded output to the destination ostream.
// @function close
int Base64EncoderUserdata::close(lua_State* L)
{
    Base64EncoderUserdata* b64eud = checkPrivateUserdata<Base64EncoderUserdata>(L, 1);
    if (!b64eud->mClosed) { b64eud->mClosed = true; b64eud->mBase64Encoder.close(); }
    return 0;
}

} // LuaPoco
