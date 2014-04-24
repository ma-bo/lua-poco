#include "Userdata.h"

namespace LuaPoco
{

int pushPocoException(lua_State* L, const Poco::Exception& e)
{
    lua_pushnil(L);
    lua_pushlstring(L, e.displayText().c_str(), e.displayText().size());
    return 2;
}

int pushUnknownException(lua_State* L)
{
    lua_pushnil(L);
    lua_pushstring(L, "unknown Exception");
    return 2;
}

// public member functions
Userdata::Userdata()
{
}

Userdata::~Userdata()
{
}

bool Userdata::isCopyable()
{
    return false;
}

bool Userdata::copyToState(lua_State *L)
{
    return false;
}

BaseType Userdata::getBaseType()
{
    return BaseType_None;
}

} // LuaPoco
