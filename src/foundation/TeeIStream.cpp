/// teeistream
// An istream interface that will copy all read data to all ostreams added to it.
// All ostreams added to the teeistream will be referenced to avoid garbage collection.
// @module teeistream
#include "TeeIStream.h"
#include "TeeOStream.h"
#include "Buffer.h"
#include <Poco/Exception.h>

int luaopen_poco_teeistream(lua_State* L)
{
    LuaPoco::TeeIStreamUserdata::registerTeeIStream(L);
    return LuaPoco::loadConstructor(L, LuaPoco::TeeIStreamUserdata::TeeIStream);
}

namespace LuaPoco
{

const char* POCO_TEEISTREAM_METATABLE_NAME = "Poco.TeeIStream.metatable";

TeeIStreamUserdata::TeeIStreamUserdata(std::istream& is) : mTeeInputStream(is)
{
}

TeeIStreamUserdata::~TeeIStreamUserdata()
{
}

std::istream& TeeIStreamUserdata::istream()
{
    return mTeeInputStream;
}

// register metatable for this class
bool TeeIStreamUserdata::registerTeeIStream(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "read", read },
        { "lines", lines },
        { "seek", seek },
        { "addStream", addStream },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_TEEISTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new teeistream userdata.
// @param the source istream to be read from, which will be copied to attached ostreams.
// @return userdata
// @function new
int TeeIStreamUserdata::TeeIStream(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    int top = lua_gettop(L);

    IStream* isud = checkPrivateUserdata<IStream>(L, firstArg);
    TeeIStreamUserdata* tisud = NULL;
    void* p = lua_newuserdata(L, sizeof *tisud);
    
    try
    {
        tisud = new(p) TeeIStreamUserdata(isud->istream());
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    // copy the value to the top of the stack, such that luaL_ref can pop it back off.
    lua_pushvalue(L, firstArg);
    tisud->mUdReference.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
    
    setupPocoUserdata(L, tisud, POCO_TEEISTREAM_METATABLE_NAME);
    return 1;
}

// metamethod infrastructure
int TeeIStreamUserdata::metamethod__gc(lua_State* L)
{
    TeeIStreamUserdata* tisud = checkPrivateUserdata<TeeIStreamUserdata>(L, 1);

    // unreference all userdata that have been added.
    for (size_t i = 0; i < tisud->mUdReference.size(); ++i)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, tisud->mUdReference[i]);
    }
    tisud->~TeeIStreamUserdata();

    return 0;
}

int TeeIStreamUserdata::metamethod__tostring(lua_State* L)
{
    TeeIStreamUserdata* tisud = checkPrivateUserdata<TeeIStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.TeeIStream (%p)", static_cast<void*>(tisud));
    
    return 1;
}
/// Adds an ostream to the teeistream.
// @param array containing ostream userdata, or one or more ostream 
// userdata passed as parameters.
// @function addStream
// @see ostream
int TeeIStreamUserdata::addStream(lua_State* L)
{
    int top = lua_gettop(L);
    TeeIStreamUserdata* tisud = checkPrivateUserdata<TeeIStreamUserdata>(L, 1);
    
    // check for the second arg to be a table, and iterate it for userdata.
    if (top > 1 && lua_istable(L, 2))
    {
        int tableIndex = 2;
        for (int i = 1; ; ++i)
        {
            lua_pushinteger(L, i);
            lua_gettable(L, tableIndex);
            if (lua_isnil(L, -1)) break;
            OStreamUserdata* osud = checkPrivateUserdata<OStreamUserdata>(L, -1);
            tisud->mTeeInputStream.addStream(osud->ostream());
            tisud->mUdReference.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
        }
    }
    // run through arguments.
    else
    {
        for (int i = 2; i <= top; ++i)
        {
            if (lua_isnil(L, i)) break;
            OStream* osud = checkPrivateUserdata<OStream>(L, i);
            tisud->mTeeInputStream.addStream(osud->ostream());
            // copy the value to the top of the stack, such that luaL_ref can pop it back off.
            lua_pushvalue(L, i);
            tisud->mUdReference.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
        }
    }
    
    return 0;
}

} // LuaPoco
