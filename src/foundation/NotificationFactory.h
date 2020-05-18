#ifndef LUA_POCO_NOTIFICATION_FACTORY_H
#define LUA_POCO_NOTIFICATION_FACTORY_H

#include "LuaPoco.h"
#include <Poco/Notification.h>
#include <Poco/AutoPtr.h>
#include "Notification.h"

namespace LuaPoco
{

class NotificationFactory
{
public:
    Poco::AutoPtr<Notification> createObject();
    bool validateObject(Poco::AutoPtr<Notification> notification);
    void activateObject(Poco::AutoPtr<Notification> notification);
    void deactivateObject(Poco::AutoPtr<Notification> notification);
    void destroyObject(Poco::AutoPtr<Notification> notification);
};

} // LuaPoco

#endif
