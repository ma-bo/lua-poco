#ifndef LUA_POCO_CONDITION_H
#define LUA_POCO_CONDITION_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/Condition.h>
#include <Poco/SharedPtr.h>

extern "C"
{

LUAPOCO_API int luaopen_poco_condition(lua_State* L);

}

namespace LuaPoco
{
class ConditionUserdata : public Userdata
{
public:
    ConditionUserdata();
    ConditionUserdata(const Poco::SharedPtr<Poco::Condition>& condition);
    virtual ~ConditionUserdata();
    virtual bool copyToState(lua_State* L);
    // constructor
    static int Condition(lua_State* L);
    static bool registerCondition(lua_State* L);
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    // userdata methods
    static int broadcast(lua_State* L);
    static int signal(lua_State* L);
    static int wait(lua_State* L);
    static int tryWait(lua_State* L);
    
    Poco::SharedPtr<Poco::Condition> mCondition;
};

} // LuaPoco

#endif
