#ifndef LUA_POCO_EVENT_H
#define LUA_POCO_EVENT_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "Poco/Event.h"
#include "Poco/SharedPtr.h"

extern "C"
{
LUAPOCO_API int luaopen_poco_event(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_EVENT_METATABLE_NAME;

class EventUserdata : public Userdata
{
public:
    EventUserdata(bool autoReset = true);
    EventUserdata(const Poco::SharedPtr<Poco::Event>& event);
    virtual ~EventUserdata();
    virtual bool copyToState(lua_State *L);
    // register metatable for this class
    static bool registerEvent(lua_State* L);
    // constructor function 
    static int Event(lua_State* L);
    
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int set(lua_State* L);
    static int tryWait(lua_State* L);
    static int wait(lua_State* L);
    static int reset(lua_State* L);
    
    Poco::SharedPtr<Poco::Event> mEvent;
};

} // LuaPoco

#endif
