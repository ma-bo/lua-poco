#include "NotificationFactory.h"
#include <cstdio>

namespace LuaPoco
{

Poco::AutoPtr<Notification> NotificationFactory::createObject()
{
    return new Notification;
}

bool NotificationFactory::validateObject(Poco::AutoPtr<Notification> notification)
{
    // this function could be used to do some sanity checking on the state, 
    // but as of now there is nothing to check.
    return true;
}

void NotificationFactory::activateObject(Poco::AutoPtr<Notification> notification)
{
    // make sure all required LuaPoco structures are prepared in the state.
    notification->setupState();
    // guarantee a clean stack before transferring data.
    lua_settop(notification->state, 0);
}

void NotificationFactory::deactivateObject(Poco::AutoPtr<Notification> notification)
{
    // clean the state in preparation for next use.
    lua_settop(notification->state, 0);
    lua_gc(notification->state, LUA_GCCOLLECT, 0);
}

void NotificationFactory::destroyObject(Poco::AutoPtr<Notification> notification)
{
    // no action is required to destruct the Notification as it will be destructed 
    // via ~Poco::AutoPtr<Notification>() when the ObjectPool vector is destructed.
}

}
