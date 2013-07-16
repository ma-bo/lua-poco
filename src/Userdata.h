#ifndef LUA_POCO_USERDATA_H
#define LUA_POCO_USERDATA_H

#include "LuaPoco.h"

namespace LuaPoco
{

enum UserdataType
{
	Userdata_None = 0,
	Userdata_File
};

enum BaseType
{
	BaseType_None = 0,
	BaseType_File
};

class Userdata
{
public:
	Userdata();
	virtual ~Userdata();
	
	UserdataType getType();
	BaseType getBaseType();

protected:
	void setType(enum UserdataType);
	void setBaseType(enum UserdataType);
	
private:
	Userdata(const Userdata& disabledCopy);
	Userdata& operator=(const Userdata& disabledAssignment);
	
	UserBaseType mBaseType;
	UserdataType mUserdataType;
};


} // LuaPoco

#endif
