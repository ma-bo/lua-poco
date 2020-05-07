#include "Userdata.h"
#include "Notification.h"

namespace LuaPoco
{

int transferNotification(lua_State* L, Poco::AutoPtr<Notification>& n)
{
    int top = lua_gettop(n->state);
    for (int i = 1; i <= top; ++i)
    {
        lua_pushvalue(n->state, i);
        if (!transferValue(L, n->state))
        {
            lua_pushnil(L);
            lua_pushfstring(L, "non-copyable value at parameter %d\n", i);
            top = 2;
            break;
        }
        lua_pop(n->state, 1);
    }
    return top;
}

Notification::Notification()
{
    state = luaL_newstate();
}

Notification::~Notification()
{
    lua_close(state);
}

bool Notification::setupState()
{
    setupPrivateUserdata(state);
    return true;
}

} // LuaPoco
