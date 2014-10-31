/// pipeostream
// An ostream interface for pipe userdata.
// @module pipeostream
#include "PipeOStream.h"
#include "Pipe.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_pipeostream(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::PipeOStreamUserdata::PipeOStream);
}

namespace LuaPoco
{

const char* POCO_PIPEOSTREAM_METATABLE_NAME = "Poco.PipeOStream.metatable";

PipeOStreamUserdata::PipeOStreamUserdata(const Poco::Pipe& p) 
    : mPipeOutputStream(p)
    , mPipeReference(0)
{
}

PipeOStreamUserdata::~PipeOStreamUserdata()
{
}

std::ostream& PipeOStreamUserdata::ostream()
{
    return mPipeOutputStream;
}

// register metatable for this class
bool PipeOStreamUserdata::registerPipeOStream(lua_State* L)
{
    struct UserdataMethod methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "write", write },
        { "seek", seek },
        { "flush", flush },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_PIPEOSTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new pipeostream userdata.
// pipeostream holds a reference to a pipe userdata to prevent the pipe from
// being garbage collected while the pipeostream is still trying to use it.
// @param pipe userdata.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see ostream
// @see pipe
int PipeOStreamUserdata::PipeOStream(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    PipeUserdata* pud = checkPrivateUserdata<PipeUserdata>(L, firstArg);
    
    PipeOStreamUserdata* posud = new(lua_newuserdata(L, sizeof *posud)) PipeOStreamUserdata(pud->mPipe);
    setupPocoUserdata(L, posud, POCO_PIPEOSTREAM_METATABLE_NAME);
    // store a reference to the PipeUserdata to prevent it from being
    // garbage collected while the PipeOutputStream is using it.
    lua_pushvalue(L, 1);
    posud->mPipeReference = luaL_ref(L, LUA_REGISTRYINDEX);
    
    return 1;
}

int PipeOStreamUserdata::metamethod__gc(lua_State* L)
{
    PipeOStreamUserdata* pud = checkPrivateUserdata<PipeOStreamUserdata>(L, 1);

    luaL_unref(L, LUA_REGISTRYINDEX, pud->mPipeReference);
    pud->~PipeOStreamUserdata();

    return 0;
}

// metamethod infrastructure
int PipeOStreamUserdata::metamethod__tostring(lua_State* L)
{
    PipeOStreamUserdata* pud = checkPrivateUserdata<PipeOStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.PipeOStream (%p)", static_cast<void*>(pud));
    
    return 1;
}

} // LuaPoco
