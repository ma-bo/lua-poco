#ifndef LUA_POCO_CHECKSUM_H
#define LUA_POCO_CHECKSUM_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "Poco/Checksum.h"

extern "C"
{
int luaopen_poco_checksum(lua_State* L);
}

namespace LuaPoco
{

class ChecksumUserdata : public Userdata
{
public:
    ChecksumUserdata(Poco::Checksum::Type t);
    virtual ~ChecksumUserdata();
    virtual UserdataType getType();
    virtual bool isCopyable();
    virtual bool copyToState(lua_State *L);
    // register metatable for this class
    static bool registerChecksum(lua_State* L);
    // constructor function 
    static int Checksum(lua_State* L);
private:
    ChecksumUserdata();
    
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int update(lua_State* L);
    static int checksum(lua_State* L);
    static int type(lua_State* L);
    
    Poco::Checksum mChecksum;
};

} // LuaPoco

#endif
