/// base64decoder
// An istream interface for inputting base64 encoded data.
// @module base64decoder
#include "Base64Decoder.h"
#include <Poco/Base64Encoder.h>
#include <Poco/Exception.h>
#include <cstring>

int luaopen_poco_base64decoder(lua_State* L)
{
    LuaPoco::Base64DecoderUserdata::registerBase64Decoder(L);
    return LuaPoco::loadConstructor(L, LuaPoco::Base64DecoderUserdata::Base64Decoder);
}

namespace LuaPoco
{

const char* POCO_BASE64DECODER_METATABLE_NAME = "Poco.Base64Decoder.metatable";

Base64DecoderUserdata::Base64DecoderUserdata(std::istream & istream, int options, int ref)
    : mBase64Decoder(istream, options)
    , mUdReference(ref)
    , mClosed(false)
{
}

Base64DecoderUserdata::~Base64DecoderUserdata()
{
}

std::istream& Base64DecoderUserdata::istream()
{
    return mBase64Decoder;
}

// register metatable for this class
bool Base64DecoderUserdata::registerBase64Decoder(lua_State* L)
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
    
    setupUserdataMetatable(L, POCO_BASE64DECODER_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new base64decoder userdata.
// @tparam userdata istream destination for base64 encoded data.
// @string[opt] options "url_encoding", "no_padding", or both separated by space or commas.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see istream
int Base64DecoderUserdata::Base64Decoder(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    IStream* is = checkPrivateUserdata<IStream>(L, firstArg);
    int options = 0;
    const char* optionsStr = lua_tostring(L, firstArg + 1);
    if (optionsStr)
    {
        if (std::strstr(optionsStr, "url_encoding")) { options |= Poco::BASE64_URL_ENCODING; }
        if (std::strstr(optionsStr, "no_padding")) { options |= Poco::BASE64_NO_PADDING; }
    }

    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    Base64DecoderUserdata* b64dud = NULL;
    void* p = lua_newuserdata(L, sizeof *b64dud);
    
    try
    {
        b64dud = new(p) Base64DecoderUserdata(is->istream(), options, ref);
    }
    catch (const std::exception& e)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        return pushException(L, e);
    }
    
    setupPocoUserdata(L, b64dud, POCO_BASE64DECODER_METATABLE_NAME);
    return 1;
}

// metamethod infrastructure
int Base64DecoderUserdata::metamethod__tostring(lua_State* L)
{
    Base64DecoderUserdata* b64dud = checkPrivateUserdata<Base64DecoderUserdata>(L, 1);
    lua_pushfstring(L, "Poco.Base64Decoder (%p)", static_cast<void*>(b64dud));
    
    return 1;
}

int Base64DecoderUserdata::metamethod__gc(lua_State* L)
{
    Base64DecoderUserdata* b64dud = checkPrivateUserdata<Base64DecoderUserdata>(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, b64dud->mUdReference);
    b64dud->~Base64DecoderUserdata();

    return 0;
}
/// 
// @type base64decoder

} // LuaPoco
