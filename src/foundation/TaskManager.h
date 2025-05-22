#ifndef LUA_POCO_TASK_MANAGER_H
#define LUA_POCO_TASK_MANAGER_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Notification.h"
#include "NotificationFactory.h"
#include <Poco/TaskManager.h>
#include <Poco/Task.h>
#include <Poco/TaskNotification.h>
#include <Poco/ObjectPool.h>
#include <Poco/ThreadPool.h>
#include <Poco/SharedPtr.h>
#include <Poco/AtomicCounter.h>
#include <Poco/NotificationQueue.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_taskmanager(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_TASK_MANAGER_METATABLE_NAME;

#define TASK_NOTIFICATION_START 1
#define TASK_NOTIFICATION_CANCELLED (1 << 2)
#define TASK_NOTIFICATION_FINISHED (1 << 3)
#define TASK_NOTIFICATION_FAILED (1 << 4)
#define TASK_NOTIFICATION_PROGRESS (1 << 5)
#define TASK_NOTIFICATION_CUSTOM (1 << 6)

class Task : public Poco::Task
{
public:
    Task(const char* taskName);
    virtual ~Task();
    virtual void runTask();
    bool prepTask(
            lua_State* L,
            int taskManagerLudIndex,
            int functionIndex,
            int firstParamIndex,
            int lastParamIndex);
    
    // Task member functions to be used via a table/metatable/lightuserdata
    //  on the public interface of a Task*
    static int lud_isCancelled(lua_State* L);
    static int lud_cancel(lua_State* L);
    static int lud_name(lua_State* L);
    static int lud_progress(lua_State* L);
    static int lud_reset(lua_State* L);
    static int lud_state(lua_State* L);

    // Task member functions to be used via a table/metatable/lightuserdata
    //  on the protected interface of a Task*
    static int lud_setState(lua_State* L);
    static int lud_setProgress(lua_State* L);
    static int lud_sleep(lua_State* L);
    static int lud_postNotification(lua_State* L);

private:
    lua_State* mState;
};

class TaskManagerContainer
{
public:
    TaskManagerContainer(int minThreads, int maxThreads, int idleTimeout, int stackSize,
                        int minPool, int maxPool);
    ~TaskManagerContainer();

    void enableTaskQueue();
    void disableTaskQueue();
    int waitDequeueNotification(lua_State* L, long milliseconds);

    // Tasks receive a raw pointer to the TaskManagerContainer which is placed in a table.
    static int lud_count(lua_State* L);
    static int lud_taskList(lua_State* L);
    static int lud_cancelAll(lua_State* L);
    // joinAll should not be called from observers, as observers are run from a ThreadPool thread.
    // static int lud_joinAll(lua_State* L);
    static int lud_start(lua_State* L);
    static int lud_enableTaskQueue(lua_State* L);
    static int lud_disableTaskQueue(lua_State* L);
    static int lud_dequeueNotification(lua_State* L);

    Poco::ObjectPool<Notification, Poco::AutoPtr<Notification>, NotificationFactory> mPool;    
    Poco::TaskManager mTaskManager;
    Poco::AtomicCounter mDestruct;
    
private:
    void onTaskStarted(Poco::TaskStartedNotification* sn);
    void onTaskCancelled(Poco::TaskCancelledNotification* cn);
    void onTaskFinished(Poco::TaskFinishedNotification* fn);
    void onTaskFailed(Poco::TaskFailedNotification* fn);
    void onTaskProgress(Poco::TaskProgressNotification* pn);
    void onTaskCustom(Notification* n);

    Poco::ThreadPool mThreadPool;
    Poco::AtomicCounter mQueueEnabled;
    Poco::NotificationQueue mQueue;
};

class TaskManagerUserdata : public Userdata
{
public:
    TaskManagerUserdata(int minThreads, int maxThreads, int idleTimeout, int stackSize,
                        int minPool, int maxPool);
    TaskManagerUserdata(Poco::SharedPtr<TaskManagerContainer>& tmc);
            
    virtual ~TaskManagerUserdata();
    // register metatable for this class
    static bool registerTaskManager(lua_State* L);
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    // constructor function 
    static int TaskManager(lua_State* L);
    
    virtual bool copyToState(lua_State *L);
    
private:
    // userdata methods
    static int count(lua_State* L);
    static int taskList(lua_State* L);
    static int cancelAll(lua_State* L);
    static int joinAll(lua_State* L);
    static int start(lua_State* L);
    static int enableTaskQueue(lua_State* L);
    static int disableTaskQueue(lua_State* L);
    static int dequeueNotification(lua_State* L);
    // member functions exposed via TaskManagerUserdata to operate on contained
    // tasks, without having to obtain a table, light userdata, and metatables.
    static int isTaskCancelled(lua_State* L);
    static int taskCancel(lua_State* L);
    static int taskName(lua_State* L);
    static int taskProgress(lua_State* L);
    static int taskReset(lua_State* L);
    static int taskState(lua_State* L);

    Poco::SharedPtr<TaskManagerContainer> mContainer;
};

} // LuaPoco

#endif
