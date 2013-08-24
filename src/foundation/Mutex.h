#ifndef LUA_POCO_Mutex_H
#define LUA_POCO_Mutex_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/Mutex.h"
#include "Poco/SharedPtr.h"

namespace LuaPoco
{

class MutexUserdata : public Userdata
{
public:
	MutexUserdata();
	MutexUserdata(const Poco::SharedPtr<Poco::Mutex>& mtx);
	virtual ~MutexUserdata();
	virtual UserdataType getType();
	virtual bool isCopyable();
	virtual bool copyToState(lua_State *L);
	// register metatable for this class
	static bool registerMutex(lua_State* L);
private:
	// constructor function 
	static int Mutex(lua_State* L);
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	static int metamethod__tostring(lua_State* L);
	
	// userdata methods
	static int lock(lua_State* L);
	static int tryLock(lua_State* L);
	static int unlock(lua_State* L);
	
	Poco::SharedPtr<Poco::Mutex> mMutex;
};

} // LuaPoco

#endif
