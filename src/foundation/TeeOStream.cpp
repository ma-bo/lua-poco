/// teeostream
// An ostream interface that will mirror writes to all ostreams added to it.
// All ostreams added to the teeostream will be referenced to avoid garbage collection.
// The teeostream interface is particularly useful for debugging writes to other ostreams like
// pipes and sockets, as they can be also sent to a file.
// @module teeostream
#include "TeeOStream.h"
#include "Buffer.h"
#include "Poco/Exception.h"

int luaopen_poco_teeostream(lua_State* L)
{
    LuaPoco::TeeOStreamUserdata::registerTeeOStream(L);
    return LuaPoco::loadConstructor(L, LuaPoco::TeeOStreamUserdata::TeeOStream);
}

namespace LuaPoco
{

const char* POCO_TEEOSTREAM_METATABLE_NAME = "Poco.TeeOStream.metatable";

TeeOStreamUserdata::TeeOStreamUserdata()
{
}

TeeOStreamUserdata::~TeeOStreamUserdata()
{
}

std::ostream& TeeOStreamUserdata::ostream()
{
    return mTeeOutputStream;
}

// register metatable for this class
bool TeeOStreamUserdata::registerTeeOStream(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "write", write },
        { "seek", seek },
        { "flush", flush },
        { "addStream", addStream },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_TEEOSTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new teeostream userdata.
// @param ...  one or more ostream's to be placed into the teeostream.
// @return userdata
// @function new
int TeeOStreamUserdata::TeeOStream(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    int top = lua_gettop(L);
    
    TeeOStreamUserdata* tosud = new(lua_newuserdata(L, sizeof *tosud)) TeeOStreamUserdata();
    setupPocoUserdata(L, tosud, POCO_TEEOSTREAM_METATABLE_NAME);

    for (int i = firstArg; i <= top; ++i)
    {
        if (lua_isnil(L, i)) break;
        OStream* osud = checkPrivateUserdata<OStream>(L, i);
        tosud->mTeeOutputStream.addStream(osud->ostream());
        // copy the value to the top of the stack, such that luaL_ref can pop it back off.
        lua_pushvalue(L, i);
        tosud->mUdReference.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
    }
    
    return 1;
}

// metamethod infrastructure
int TeeOStreamUserdata::metamethod__gc(lua_State* L)
{
    TeeOStreamUserdata* tosud = checkPrivateUserdata<TeeOStreamUserdata>(L, 1);

    // unreference all userdata that have been added.
    for (size_t i = 0; i < tosud->mUdReference.size(); ++i)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, tosud->mUdReference[i]);
    }
    tosud->~TeeOStreamUserdata();

    return 0;
}

int TeeOStreamUserdata::metamethod__tostring(lua_State* L)
{
    TeeOStreamUserdata* tosud = checkPrivateUserdata<TeeOStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.TeeOStream (%p)", static_cast<void*>(tosud));
    
    return 1;
}
/// Adds an ostream to the teeostream.
// @param array containing ostream userdata, or one or more ostream 
// userdata passed as parameters.
// @function addStream
// @see ostream
int TeeOStreamUserdata::addStream(lua_State* L)
{
    int top = lua_gettop(L);
    TeeOStreamUserdata* tosud = checkPrivateUserdata<TeeOStreamUserdata>(L, 1);
    
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
            tosud->mTeeOutputStream.addStream(osud->ostream());
            tosud->mUdReference.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
        }
    }
    // run through arguments.
    else
    {
        for (int i = 2; i <= top; ++i)
        {
            if (lua_isnil(L, i)) break;
            OStream* osud = checkPrivateUserdata<OStream>(L, i);
            tosud->mTeeOutputStream.addStream(osud->ostream());
            // copy the value to the top of the stack, such that luaL_ref can pop it back off.
            lua_pushvalue(L, i);
            tosud->mUdReference.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
        }
    }
    
    return 0;
}

} // LuaPoco
