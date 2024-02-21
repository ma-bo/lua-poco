/// random
//  Random implements a pseudo random number generator (PRNG).
// The PRNG is a nonlinear additive feedback random number generator using 256 bytes of state
// information and a period of up to 2^69.
// @module random
#include "Random.h"
#include <Poco/Exception.h>

int luaopen_poco_random(lua_State* L)
{
    LuaPoco::RandomUserdata::registerRandom(L);
    return LuaPoco::loadConstructor(L, LuaPoco::RandomUserdata::Random);
}

namespace LuaPoco
{

const char* POCO_RANDOM_METATABLE_NAME = "Poco.Random.metatable";

RandomUserdata::RandomUserdata(int stateSize)
    : mRandom(stateSize)
{
}

RandomUserdata::~RandomUserdata()
{
}

// register metatable for this class
bool RandomUserdata::registerRandom(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "next", next },
        { "nextNumber", nextNumber },
        { "nextByte", nextByte },
        { "nextBool", nextBool },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_RANDOM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new random userdata.
// @tparam[opt] number stateSize (default: 256) state buffer size: 8, 16, 32, 64, 128, or 256.
// @tparam[opt] number seed seed for prng, if unspecified an internal RandomNumberStream is used.
// @return userdata or nil. (error)
// @return error message.
// @function new
int RandomUserdata::Random(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    
    lua_Integer stateSize = 256;
    lua_Integer seed = lua_isnumber(L, firstArg + 1) ? lua_tointeger(L, firstArg + 1) : 0;
    
    if (lua_isnumber(L, firstArg)) { stateSize = lua_tointeger(L, firstArg); }
    if (stateSize != 256 && stateSize != 128 && stateSize != 64 && stateSize != 32 &&
        stateSize != 16 && stateSize != 8)
    {
        lua_pushnil(L); lua_pushfstring(L, "invalid stateSize: %d", stateSize);
        return 2;
    }
    
    try
    {
        RandomUserdata* rud = new(lua_newuserdata(L, sizeof *rud))
            RandomUserdata(stateSize);
        setupPocoUserdata(L, rud, POCO_RANDOM_METATABLE_NAME);

        if (seed) { rud->mRandom.seed(seed); }
        else { rud->mRandom.seed(); }
        
        rv = 1;
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        
    return rv;
}

// metamethod infrastructure
int RandomUserdata::metamethod__tostring(lua_State* L)
{
    RandomUserdata* rud = checkPrivateUserdata<RandomUserdata>(L, 1);
    lua_pushfstring(L, "Poco.Random (%p)", static_cast<void*>(rud));
    
    return 1;
}

int RandomUserdata::metamethod__gc(lua_State* L)
{
    RandomUserdata* rud = checkPrivateUserdata<RandomUserdata>(L, 1);
    rud->~RandomUserdata();

    return 0;
}
/// 
// @type random

/// Returns the next 31-bit pseudo random number. (modulo n, if supplied)
// @tparam[opt] integer n modulo value.
// @function next
int RandomUserdata::next(lua_State* L)
{
    RandomUserdata* rud = checkPrivateUserdata<RandomUserdata>(L, 1);
    lua_Integer rval = 0;
    
    if (lua_isnumber(L, 2))
        { rval = static_cast<lua_Integer>(rud->mRandom.next(lua_tointeger(L, 2))); }
    else { rval = rud->mRandom.next(); }
    
    lua_pushinteger(L, rval);
    return 1;
}

/// Returns the next pseudo random number between 0.0 and 1.0 as a double.
// @function nextNumber
int RandomUserdata::nextNumber(lua_State* L)
{
    RandomUserdata* rud = checkPrivateUserdata<RandomUserdata>(L, 1);
    lua_pushnumber(L, static_cast<lua_Number>(rud->mRandom.nextDouble()));
    return 1;
}

/// Returns the next pseudo random byte.
// @function nextByte
int RandomUserdata::nextByte(lua_State* L)
{
    RandomUserdata* rud = checkPrivateUserdata<RandomUserdata>(L, 1);
    unsigned char rval = static_cast<unsigned char>(rud->mRandom.nextChar());
    lua_pushinteger(L, static_cast<lua_Integer>(rval));
    return 1;
}

/// Returns the next pseudo random boolean value.
// @function nextBool
int RandomUserdata::nextBool(lua_State* L)
{
    RandomUserdata* rud = checkPrivateUserdata<RandomUserdata>(L, 1);
    lua_pushboolean(L, static_cast<int>(rud->mRandom.nextBool()));
    return 1;
}

} // LuaPoco

