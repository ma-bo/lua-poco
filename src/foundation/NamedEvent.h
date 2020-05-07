#ifndef LUA_POCO_NAMEDEVENT_H
#define LUA_POCO_NAMEDEVENT_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/NamedEvent.h"
#include <string>

extern "C"
{
LUAPOCO_API int luaopen_poco_namedevent(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_NAMEDEVENT_METATABLE_NAME;

class NamedEventUserdata : public Userdata
{
public:
    NamedEventUserdata(const std::string& name);
    virtual ~NamedEventUserdata();
    // register metatable for this class
    static bool registerNamedEvent(lua_State* L);
    // constructor function 
    static int NamedEvent(lua_State* L);
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int set(lua_State* L);
    static int wait(lua_State* L);
    
    Poco::NamedEvent mNamedEvent;
};

} // LuaPoco

#endif
