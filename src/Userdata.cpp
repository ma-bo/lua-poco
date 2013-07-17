#include "Userdata.h"

namespace LuaPoco
{

// public member functions
Userdata::Userdata() : 
	mBaseType(BaseType_None), mUserdataType(Userdata_None)
{
}

Userdata::~Userdata()
{
	mBaseType = BaseType_None;
	mUserdataType = Userdata_None;
}

UserdataType Userdata::getType()
{
	return mUserdataType;
}

BaseType Userdata::getBaseType()
{
	return mBaseType;
}


void Userdata::setType(UserdataType type)
{
	mUserdataType = type;
}

void Userdata::setBaseType(BaseType type)
{
	mBaseType = type;
}

} // LuaPoco
