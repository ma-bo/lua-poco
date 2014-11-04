/// pipeistream
// An istream interface for pipe userdata.
// @module pipeistream
#include "PipeIStream.h"
#include "Pipe.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_pipeistream(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::PipeIStreamUserdata::PipeIStream);
}

namespace LuaPoco
{

const char* POCO_PIPEISTREAM_METATABLE_NAME = "Poco.PipeIStream.metatable";

PipeIStreamUserdata::PipeIStreamUserdata(const Poco::Pipe& p) 
    : mPipeInputStream(p)
    , mPipeReference(0)
{
}

PipeIStreamUserdata::~PipeIStreamUserdata()
{
}

std::istream& PipeIStreamUserdata::istream()
{
    return mPipeInputStream;
}

// register metatable for this class
bool PipeIStreamUserdata::registerPipeIStream(lua_State* L)
{
    struct UserdataMethod methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "read", read },
        { "lines", lines },
        { "seek", seek },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_PIPEISTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new pipeistream userdata.
// pipeistream holds a reference to a pipe userdata to prevent the pipe from
// being garbage collected while the pipeistream is still trying to use it.
// @param pipe userdata.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see istream
// @see pipe
int PipeIStreamUserdata::PipeIStream(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    PipeUserdata* pud = checkPrivateUserdata<PipeUserdata>(L, firstArg);
    
    PipeIStreamUserdata* posud = new(lua_newuserdata(L, sizeof *posud)) PipeIStreamUserdata(pud->mPipe);
    setupPocoUserdata(L, posud, POCO_PIPEISTREAM_METATABLE_NAME);
    // store a reference to the PipeUserdata to prevent it from being
    // garbage collected while the PipeOutputStream is using it.
    lua_pushvalue(L, 1);
    posud->mPipeReference = luaL_ref(L, LUA_REGISTRYINDEX);
    
    return 1;
}

int PipeIStreamUserdata::metamethod__gc(lua_State* L)
{
    PipeIStreamUserdata* pud = checkPrivateUserdata<PipeIStreamUserdata>(L, 1);

    luaL_unref(L, LUA_REGISTRYINDEX, pud->mPipeReference);
    pud->~PipeIStreamUserdata();

    return 0;
}

// metamethod infrastructure
int PipeIStreamUserdata::metamethod__tostring(lua_State* L)
{
    PipeIStreamUserdata* pud = checkPrivateUserdata<PipeIStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.PipeIStream (%p)", static_cast<void*>(pud));
    
    return 1;
}

} // LuaPoco
