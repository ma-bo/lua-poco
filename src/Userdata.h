#ifndef LUA_POCO_USERDATA_H
#define LUA_POCO_USERDATA_H

#include "LuaPoco.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

// generic functions to reduce the amount of copy and paste code.
int pushPocoException(lua_State* L, const Poco::Exception& e);
int pushUnknownException(lua_State* L);

// base class for all userdata
// the idea is that it will be possible to pass a userdata to another 
// userdata's method, and these types this will enable base class 
// compatability testing.
class Userdata
{
public:
    Userdata();
    virtual ~Userdata();
    virtual bool copyToState(lua_State *L);

private:
    Userdata(const Userdata& disabledCopy);
    Userdata& operator=(const Userdata& disabledAssignment);
};


} // LuaPoco

#endif
