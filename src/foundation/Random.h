#ifndef LUA_POCO_RANDOM_H
#define LUA_POCO_RANDOM_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/Random.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_random(lua_State* L);
}

namespace LuaPoco
{

class RandomUserdata : public Userdata
{
public:
    RandomUserdata(int stateSize);
    virtual ~RandomUserdata();
    // register metatable for this class
    static bool registerRandom(lua_State* L);
    // constructor function 
    static int Random(lua_State* L);
    
    Poco::Random mRandom;
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    static int metamethod__gc(lua_State* L);
    // methods
    static int next(lua_State* L);
    static int nextNumber(lua_State* L);
    static int nextByte(lua_State* L);
    static int nextBool(lua_State* L);
};

} // LuaPoco

#endif
