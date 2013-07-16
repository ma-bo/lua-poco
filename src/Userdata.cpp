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
	return mUserdataType
}

BaseType getBaseType()
{
	return mBaseType;
}

// inherited member functions
void Userdata::setType(enum UserdataType userdataType)
{
	mUserdataType = userdataType;
}

void Userdata::setBaseType(BaseType baseType)
{
	mBaseType = baseType;
}

} // LuaPoco
