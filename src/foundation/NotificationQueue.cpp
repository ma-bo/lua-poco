/// NotificationQueue provides a way to implement asynchronous notifications.
// There are two recommended ways of using the NotificationQueue:
//
// 1. A main/controller thread spawns several workers threads, and the multiple workers 
// dequeue notifications from the controller.
//
// 2. A main/controller thread spawns several worker threads, and the controller thread
// dequeues messages from the workers.
//
// It is quite possible to do both scenarios at the same time via two NotificationQueues.
//
// Note: notificationqueue userdata are sharable between threads.
// @module notificationqueue

#include "NotificationQueue.h"
#include <Poco/Exception.h>
#include "StateTransfer.h"

int luaopen_poco_notificationqueue(lua_State* L)
{
    LuaPoco::NotificationQueueUserdata::registerNotificationQueue(L);
    return LuaPoco::loadConstructor(L, LuaPoco::NotificationQueueUserdata::NotificationQueue);
}

namespace LuaPoco
{

const char* POCO_NOTIFICATIONQUEUE_METATABLE_NAME = "Poco.NotificationQueue.metatable";

NotificationQueueUserdata::NotificationQueueUserdata() :
    mQueue(new Poco::NotificationQueue()),
    mPool(new Poco::ObjectPool<Notification, Poco::AutoPtr<Notification>, NotificationFactory>(1, 0xFFFFFFFF))
{
}

// construct new userdata from existing SharedPtr
NotificationQueueUserdata::NotificationQueueUserdata(
    const Poco::SharedPtr<Poco::NotificationQueue>& nq,
    const Poco::SharedPtr<Poco::ObjectPool<Notification, Poco::AutoPtr<Notification>, NotificationFactory> >& op) : 
    mQueue(nq),
    mPool(op)
{
}

NotificationQueueUserdata::~NotificationQueueUserdata()
{
}

bool NotificationQueueUserdata::copyToState(lua_State *L)
{
    LuaPoco::NotificationQueueUserdata::registerNotificationQueue(L);
    NotificationQueueUserdata* nqud = new(lua_newuserdata(L, sizeof *nqud)) NotificationQueueUserdata(mQueue, mPool);
    setupPocoUserdata(L, nqud, POCO_NOTIFICATIONQUEUE_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool NotificationQueueUserdata::registerNotificationQueue(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "clear", clear },
        { "dequeue", dequeue },
        { "empty", empty },
        { "enqueue", enqueue },
        { "hasIdleThreads", hasIdleThreads },
        { "size", size },
        { "waitDequeue", waitDequeue},
        { "wakeUpAll", wakeUpAll },
        { NULL, NULL}
    };

    setupUserdataMetatable(L, POCO_NOTIFICATIONQUEUE_METATABLE_NAME, methods);
    return true;
}

/// create a new notificationqueue userdata.
// @return userdata or nil. (error)
// @return error message.
// @function new
int NotificationQueueUserdata::NotificationQueue(lua_State* L)
{
    int rv = 0;

    try
    {
        NotificationQueueUserdata* nqud = new(lua_newuserdata(L, sizeof *nqud)) NotificationQueueUserdata();
        setupPocoUserdata(L, nqud, POCO_NOTIFICATIONQUEUE_METATABLE_NAME);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        rv = pushPocoException(L, e);
    }
    catch (...)
    {
        rv = pushUnknownException(L);
    }
    return rv;
}

///
// @type notificationqueue

// metamethod infrastructure
int NotificationQueueUserdata::metamethod__tostring(lua_State* L)
{
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.NotificationQueue (%p)", static_cast<void*>(nqud));
    return 1;
}

// userdata methods

///
// @type notificationqueue

/// Clear the notificationqueue of notifications.
// @function clear
int NotificationQueueUserdata::clear(lua_State* L)
{
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    nqud->mQueue->clear();

    return 0;
}

/// Dequeue a notification from the notificationqueue.
// This function is non-blocking and will return nil 
// immediately if there are no notifications in the queue.
// @return nil or string (notification type)
// @return ... one or more values that were supplied to enqueue.
// @function dequeue
int NotificationQueueUserdata::dequeue(lua_State* L)
{
    int rv = 1;
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    Poco::AutoPtr<Notification> notification(static_cast<Notification*>(nqud->mQueue->dequeueNotification()));
    
    if (!notification.isNull())
        rv = transferNotification(L, notification);
    else
        lua_pushnil(L);
    
    return rv;
}

/// Check if notificationqueue is empty.
// @return boolean indicating if the NotificationQueue is empty.
// @function empty
int NotificationQueueUserdata::empty(lua_State* L)
{
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    lua_pushboolean(L, nqud->mQueue->empty());
    return 1;
}

/// queues a notification to the notificationqueue.
// @string notificationtype the identity of notification being sent.
// @param ... one or more values that can be copied between states.
// These include any copyable poco userdata, tables, and base Lua types.
// Function values will lose any upvalues they may have stored.
// @return nil on failure (invalid parameters) or true.
// @function enqueue
int NotificationQueueUserdata::enqueue(lua_State* L)
{
    int rv = 1;
    int top = lua_gettop(L);
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    luaL_checkany(L, 2);
    luaL_checkany(L, 3);
    
    Poco::AutoPtr<Notification> notification(nqud->mPool->borrowObject());
    for (int i = 2; i <= top; ++i)
    {
        // transferValue requires value be at the top of the stack.
        lua_pushvalue(L, i);
        if (!transferValue(notification->state, L))
        {
            lua_pushnil(L);
            lua_pushfstring(L, "non-copyable value at parameter %d.\n", i);
            nqud->mPool->returnObject(notification);
            return 2;
        }
        lua_pop(L, 1);
    }
    
    nqud->mQueue->enqueueNotification(notification);
    
    lua_pushboolean(L, 1);
    
    return rv;
}

/// Checks if the notificationqueue has threads blocked waiting for notifications.
// @return boolean indicating if there are idle threads or not.
// @function hasIdleThreads
int NotificationQueueUserdata::hasIdleThreads(lua_State* L)
{
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    lua_pushboolean(L, nqud->mQueue->hasIdleThreads());
    return 1;
}

/// Gets the number of pending notifications in the queue.
// @return integer indicating the number of notifications pending in the queue.
// @function size
int NotificationQueueUserdata::size(lua_State* L)
{
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    lua_pushinteger(L, nqud->mQueue->size());
    return 1;
}

/// Dequeue a notification from the notificationqueue.
// This function does not block.
// @int[opt] timeout timeout value in milliseconds to block waiting for notification.
// if parameter is not supplied, waitDequeue will block indefinitely waiting for a notification.
// @return nil or string (notification type)
// @return ... one or more values that were supplied to enqueue.
// @function waitDequeue
int NotificationQueueUserdata::waitDequeue(lua_State* L)
{
    int rv = 1;
    int top = lua_gettop(L);
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    Poco::AutoPtr<Notification> notification;
    
    if (top == 2)
    {
        long waitMs = static_cast<long>(luaL_checknumber(L, 2));
        if (waitMs < 0) waitMs = 0;
        notification = static_cast<Notification*>(nqud->mQueue->waitDequeueNotification(waitMs));
    }
    else
    {
        notification = static_cast<Notification*>(nqud->mQueue->waitDequeueNotification());
    }
    
    if (!notification.isNull())
    {
        rv = transferNotification(L, notification);
        nqud->mPool->returnObject(notification);
    }
    else
        lua_pushnil(L);
    
    return rv;
}

/// Wakes up all threads that will block on the queue.
// Any thread blocking on the queue will fail to dequeue a message.
// @function wakeUpAll
int NotificationQueueUserdata::wakeUpAll(lua_State* L)
{
    NotificationQueueUserdata* nqud = checkPrivateUserdata<NotificationQueueUserdata>(L, 1);
    nqud->mQueue->wakeUpAll();
    return 0;
}

} // LuaPoco
