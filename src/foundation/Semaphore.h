#ifndef LUA_POCO_SEMAPHORE_H
#define LUA_POCO_SEMAPHORE_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/Semaphore.h"
#include "Poco/SharedPtr.h"

namespace LuaPoco
{

class SemaphoreUserdata : public Userdata
{
public:
	SemaphoreUserdata(int n);
	SemaphoreUserdata(int n, int max);
	SemaphoreUserdata(const Poco::SharedPtr<Poco::Semaphore>& sem);
	virtual ~SemaphoreUserdata();
	virtual UserdataType getType();
	virtual bool isCopyable();
	virtual bool copyToState(lua_State *L);
	// register metatable for this class
	static bool registerSemaphore(lua_State* L);
private:
	// constructor function 
	static int Semaphore(lua_State* L);
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	static int metamethod__tostring(lua_State* L);
	
	// userdata methods
	static int set(lua_State* L);
	static int tryWait(lua_State* L);
	static int wait(lua_State* L);
	
	Poco::SharedPtr<Poco::Semaphore> mSemaphore;
};

} // LuaPoco

#endif
