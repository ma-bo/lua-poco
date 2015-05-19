#ifndef LUA_POCO_NOTIFICATION_H
#define LUA_POCO_NOTIFICATION_H

#include "LuaPoco.h"
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

} // LuaPoco

#endif
