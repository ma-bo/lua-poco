#include "Userdata.h"

namespace LuaPoco
{

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
