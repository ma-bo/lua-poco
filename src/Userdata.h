#ifndef LUA_POCO_USERDATA_H
#define LUA_POCO_USERDATA_H

#include "LuaPoco.h"

namespace LuaPoco
{

enum UserdataType
{
	Userdata_None = 0,
	Userdata_File,
	Userdata_Timestamp,
	Userdata_Int64
};

enum BaseType
{
	BaseType_None = 0,
};

// base class for all userdata
// the idea is that it will be possible to pass a userdata to another 
// userdata's method, and these types this will enable base class 
// compatability testing.
class Userdata
{
public:
	Userdata();
	virtual ~Userdata();
	
	virtual bool isCopyable();
	virtual bool copyToState(lua_State *L);
	virtual UserdataType getType() = 0;
	virtual BaseType getBaseType();

private:
	Userdata(const Userdata& disabledCopy);
	Userdata& operator=(const Userdata& disabledAssignment);
};


} // LuaPoco

#endif
