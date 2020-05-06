/// deflatingostream
// An ostream filter for deflating data.
// @module deflatingostream
#include "DeflatingOStream.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_deflatingostream(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::DeflatingOStreamUserdata::DeflatingOStream);
}

namespace LuaPoco
{

const char* POCO_DEFLATINGOSTREAM_METATABLE_NAME = "Poco.DeflatingOStream.metatable";

DeflatingOStreamUserdata::DeflatingOStreamUserdata(std::ostream& ostream
                                                    , Poco::DeflatingStreamBuf::StreamType type
                                                    , int level
                                                    , int ref)
    : mDeflatingOutputStream(ostream, type, level)
    , mUdReference(ref)
{
}

DeflatingOStreamUserdata::DeflatingOStreamUserdata(std::ostream& ostream
                                                    , int windowBits
                                                    , int level,
                                                    int ref)
    : mDeflatingOutputStream(ostream, windowBits, level)
    , mUdReference(ref)
{
}

DeflatingOStreamUserdata::~DeflatingOStreamUserdata()
{
}

std::ostream& DeflatingOStreamUserdata::ostream()
{
    return mDeflatingOutputStream;
}

// register metatable for this class
bool DeflatingOStreamUserdata::registerDeflatingOStream(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "write", write },
        { "flush", flush },
        { "seek", seek },
        { "close", close},
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_DEFLATINGOSTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new deflatingostream userdata.
// deflatingostream holds a reference to another ostream userdata to prevent the userdata from
// being garbage collected while the deflatingostream is still trying to use it.
// @param ostream userdata.
// @param val a number for windowBits or a string for algorithm type.
//
// "STREAM_ZLIB", or "STREAM_GZIP".
// @int[opt] level 0 for no compression, 1 for fastest, 9 for best compression, (-1 level is default if no parameter is passed.)
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see ostream
int DeflatingOStreamUserdata::DeflatingOStream(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    OStream* os = checkPrivateUserdata<OStream>(L, firstArg);
    
    int level = lua_isnumber(L, firstArg + 2) ? lua_tointeger(L, firstArg + 2) : -1;
    if (level < -1 || level > 9)
    {
        lua_pushnil(L);
        lua_pushstring(L, "invalid level.");
        return 2;
    }
    
    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    DeflatingOStreamUserdata* dosud = NULL;
    if (lua_isnumber(L, firstArg + 1))
    {
        int num = static_cast<int>(luaL_checkinteger(L, firstArg + 1));
        dosud = new(lua_newuserdata(L, sizeof *dosud)) DeflatingOStreamUserdata(os->ostream(), num, level, ref);
    }
    else
    {
        Poco::DeflatingStreamBuf::StreamType type = Poco::DeflatingStreamBuf::STREAM_ZLIB;
        const char* mode = luaL_checkstring(L, firstArg + 1);
        
        if (std::strcmp(mode, "STREAM_ZLIB") == 0) type = Poco::DeflatingStreamBuf::STREAM_ZLIB;
        else if (std::strcmp(mode, "STREAM_GZIP") == 0) type = Poco::DeflatingStreamBuf::STREAM_GZIP;
        
        dosud = new(lua_newuserdata(L, sizeof *dosud)) DeflatingOStreamUserdata(os->ostream(), type, level, ref);
    }
    setupPocoUserdata(L, dosud, POCO_DEFLATINGOSTREAM_METATABLE_NAME);
    
    return 1;
}

int DeflatingOStreamUserdata::metamethod__gc(lua_State* L)
{
    DeflatingOStreamUserdata* dosud = checkPrivateUserdata<DeflatingOStreamUserdata>(L, 1);
    
    luaL_unref(L, LUA_REGISTRYINDEX, dosud->mUdReference);
    dosud->~DeflatingOStreamUserdata();
    
    return 0;
}

// metamethod infrastructure
int DeflatingOStreamUserdata::metamethod__tostring(lua_State* L)
{
    DeflatingOStreamUserdata* dosud = checkPrivateUserdata<DeflatingOStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.DeflatingOStream (%p)", static_cast<void*>(dosud));
    
    return 1;
}

// methods

/// Finalizes ostream.
// Must be called to ensure all data is properly written to the target ostream.
// @function close
int DeflatingOStreamUserdata::close(lua_State* L)
{
    DeflatingOStreamUserdata* dosud = checkPrivateUserdata<DeflatingOStreamUserdata>(L, 1);
    dosud->mDeflatingOutputStream.close();
    
    return 0;
}

} // LuaPoco
