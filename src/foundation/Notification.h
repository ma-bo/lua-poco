#ifndef LUA_POCO_NOTIFICATION_H
#define LUA_POCO_NOTIFICATION_H

#include "LuaPoco.h"
#include "StateTransfer.h"
#include "Poco/Notification.h"

namespace LuaPoco
{

class Notification : public Poco::Notification
{
public:
    Notification();
    ~Notification();
    bool setupState();
    lua_State* state;
};

// utility function for transferring a Notification to a destination lua_State.
int transferNotification(lua_State* L, Poco::AutoPtr<Notification>& n);

} // LuaPoco

#endif
