#ifndef LUA_POCO_NAMEDEVENT_H
#define LUA_POCO_NAMEDEVENT_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/NamedEvent.h"
#include <string>

namespace LuaPoco
{

class NamedEventUserdata : public Userdata
{
public:
	NamedEventUserdata(const std::string& name);
	virtual ~NamedEventUserdata();
	virtual UserdataType getType();
	// register metatable for this class
	static bool registerNamedEvent(lua_State* L);
private:
	// constructor function 
	static int NamedEvent(lua_State* L);
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	static int metamethod__tostring(lua_State* L);
	
	// userdata methods
	static int set(lua_State* L);
	static int wait(lua_State* L);
	
	Poco::NamedEvent mNamedEvent;
};

} // LuaPoco

#endif
