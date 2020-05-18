#ifndef LUA_POCO_NOTIFICATION_QUEUE_H
#define LUA_POCO_NOTIFICATION_QUEUE_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/SharedPtr.h>
#include <Poco/NotificationQueue.h>
#include <Poco/ObjectPool.h>
#include "Notification.h"
#include "NotificationFactory.h"

extern "C"
{
LUAPOCO_API int luaopen_poco_notificationqueue(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_NOTIFICATIONQUEUE_METATABLE_NAME;

class NotificationQueueUserdata : public Userdata
{
public:
    NotificationQueueUserdata();
    NotificationQueueUserdata(
        const Poco::SharedPtr<Poco::NotificationQueue>& nq,
        const Poco::SharedPtr<Poco::ObjectPool<Notification, Poco::AutoPtr<Notification>, NotificationFactory> >& op);
    virtual ~NotificationQueueUserdata();
    virtual bool copyToState(lua_State *L);
    // register metatable for this class
    static bool registerNotificationQueue(lua_State* L);
    // constructor function 
    static int NotificationQueue(lua_State* L);
    
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int clear(lua_State* L);
    static int dequeue(lua_State* L);
    static int empty(lua_State* L);
    static int enqueue(lua_State* L);
    static int hasIdleThreads(lua_State* L);
    static int size(lua_State* L);
    static int waitDequeue(lua_State* L);
    static int wakeUpAll(lua_State* L);
    
    
    Poco::SharedPtr<Poco::NotificationQueue> mQueue;
    Poco::SharedPtr<Poco::ObjectPool<Notification, Poco::AutoPtr<Notification>, NotificationFactory> > mPool;
};

} // LuaPoco

#endif
