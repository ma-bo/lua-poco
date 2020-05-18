/// inflatingostream
// An ostream filter for inflating data.
// @module inflatingostream
#include "InflatingOStream.h"
#include <Poco/Exception.h>
#include <cstring>

int luaopen_poco_inflatingostream(lua_State* L)
{
    LuaPoco::InflatingOStreamUserdata::registerInflatingOStream(L);
    return LuaPoco::loadConstructor(L, LuaPoco::InflatingOStreamUserdata::InflatingOStream);
}

namespace LuaPoco
{

const char* POCO_INFLATINGOSTREAM_METATABLE_NAME = "Poco.InflatingOStream.metatable";

InflatingOStreamUserdata::InflatingOStreamUserdata(std::ostream& ostream
                                                    , Poco::InflatingStreamBuf::StreamType type
                                                    , int ref)
    : mInflatingOutputStream(ostream, type)
    , mUdReference(ref)
{
}

InflatingOStreamUserdata::InflatingOStreamUserdata(std::ostream& ostream
                                                    , int windowBits
                                                    , int ref)
    : mInflatingOutputStream(ostream, windowBits)
    , mUdReference(ref)
{
}

InflatingOStreamUserdata::~InflatingOStreamUserdata()
{
}

std::ostream& InflatingOStreamUserdata::ostream()
{
    return mInflatingOutputStream;
}

// register metatable for this class
bool InflatingOStreamUserdata::registerInflatingOStream(lua_State* L)
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
    
    setupUserdataMetatable(L, POCO_INFLATINGOSTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new inflatingostream userdata.
// inflatingostream holds a reference to another ostream userdata to prevent the userdata from
// being garbage collected while the inflatingostream is still trying to use it.
// @param ostream userdata.
// @param val a number for windowBits or a string for algorithm type.
//
// "STREAM_ZLIB", "STREAM_GZIP", "STREAM_ZIP" (same as STREAM_ZLIB but no checksum is performed.)
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see ostream
int InflatingOStreamUserdata::InflatingOStream(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    OStream* os = checkPrivateUserdata<OStream>(L, firstArg);
    
    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    InflatingOStreamUserdata* oosud = NULL;
    if (lua_isnumber(L, firstArg + 1))
    {
        int num = static_cast<int>(luaL_checkinteger(L, firstArg + 1));
        oosud = new(lua_newuserdata(L, sizeof *oosud)) InflatingOStreamUserdata(os->ostream(), num, ref);
    }
    else
    {
        Poco::InflatingStreamBuf::StreamType type = Poco::InflatingStreamBuf::STREAM_ZLIB;
        const char* mode = luaL_checkstring(L, firstArg + 1);
        
        if (std::strcmp(mode, "STREAM_ZLIB") == 0) type = Poco::InflatingStreamBuf::STREAM_ZLIB;
        else if (std::strcmp(mode, "STREAM_GZIP") == 0) type = Poco::InflatingStreamBuf::STREAM_GZIP;
        else if (std::strcmp(mode, "STREAM_GZIP") == 0) type = Poco::InflatingStreamBuf::STREAM_ZIP;
        
        oosud = new(lua_newuserdata(L, sizeof *oosud)) InflatingOStreamUserdata(os->ostream(), type, ref);
    }
    setupPocoUserdata(L, oosud, POCO_INFLATINGOSTREAM_METATABLE_NAME);
    
    return 1;
}

int InflatingOStreamUserdata::metamethod__gc(lua_State* L)
{
    InflatingOStreamUserdata* oosud = checkPrivateUserdata<InflatingOStreamUserdata>(L, 1);
    
    luaL_unref(L, LUA_REGISTRYINDEX, oosud->mUdReference);
    oosud->~InflatingOStreamUserdata();
    
    return 0;
}

// metamethod infrastructure
int InflatingOStreamUserdata::metamethod__tostring(lua_State* L)
{
    InflatingOStreamUserdata* oosud = checkPrivateUserdata<InflatingOStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.InflatingOStream (%p)", static_cast<void*>(oosud));
    
    return 1;
}

// methods

/// Finalizes ostream.
// Must be called to ensure all data is properly written to the target ostream.
// @function close
int InflatingOStreamUserdata::close(lua_State* L)
{
    InflatingOStreamUserdata* oosud = checkPrivateUserdata<InflatingOStreamUserdata>(L, 1);
    oosud->mInflatingOutputStream.close();
    
    return 0;
}

} // LuaPoco
