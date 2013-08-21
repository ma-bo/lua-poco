#ifndef LUA_POCO_NAMEDMUTEX_H
#define LUA_POCO_NAMEDMUTEX_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/NamedMutex.h"
#include <string>

namespace LuaPoco
{

class NamedMutexUserdata : public Userdata
{
public:
	NamedMutexUserdata(const std::string& name);
	virtual ~NamedMutexUserdata();
	virtual UserdataType getType();
	// register metatable for this class
	static bool registerNamedMutex(lua_State* L);
private:
	// constructor function 
	static int NamedMutex(lua_State* L);
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	static int metamethod__tostring(lua_State* L);
	
	// userdata methods
	static int lock(lua_State* L);
	static int tryLock(lua_State* L);
	static int unlock(lua_State* L);
	
	Poco::NamedMutex mNamedMutex;
};

} // LuaPoco

#endif
