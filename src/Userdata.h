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
	void setType(UserdataType type);
	void setBaseType(BaseType type);
	
private:
	Userdata(const Userdata& disabledCopy);
	Userdata& operator=(const Userdata& disabledAssignment);
	
	BaseType mBaseType;
	UserdataType mUserdataType;
};


} // LuaPoco

#endif
