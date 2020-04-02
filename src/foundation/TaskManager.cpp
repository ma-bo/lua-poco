/// TaskManager permits running  multiple functions or chunks of isolated Lua code on a threadpool.
// Lua code is loaded into its own private Lua state, which makes multithreading convenient.
// Each of these isolated Lua states are called Tasks, which are managed by the TaskManager.
//
// Tasks are monitored are monitored by Observers.
// Observers receive notifications that indicate the status of each Task.
//
// Notification types:
//
//      "started": The Task has begun running on the TaskManager.
//
//      "cancelled": The Task has been cancelled, and will soon finish.
//
//      "finished": The Task has completed its work and has finished running.
//
//      "failed": The Task encountered an error and will terminate.
//
//      "progress": The Task is reporting progress towards completion as a percentage.
//
//      "custom": The Task is posting a custom data notification that is understood by the Observer.
//
// Note: taskmanager userdata are sharable between threads.
// @module taskmanager

/// TaskManagerSettings table is optionally supplied to the taskmanager constructor to provide
// non-default settings.
// @table TaskManagerSettings
// @field minThreads Minimum number of threads in the thread pool to keep warm, waiting for tasks.
// @field maxThreads Maximum number of threads made available to run tasks on.
// @field idleTime Time permitted in seconds for a threads above the minimum to be idle without tasks running before it is shut down.
// @field stackSize The stack size for the native OS thread.
// @field minNotificationPool Number of Notifications to keep warm in an object pool.
// @field maxNotificationPool Maximum number of Notifications permitted concurrently in flight.

/// @table TaskNotification
// @field task light userdata value for the task.
// @field type "started", "cancelled", "finished", "failed", "progress", or "custom".
// @field code number value specifing the error code of a failed task.
// @field message string value specifying the error message of a failed task.
// @field progress number value specifying the progress as a percentage from 0.0 to 100.0.

#include "TaskManager.h"
#include "Poco/Exception.h"
#include "Poco/TaskNotification.h"
#include "Poco/Observer.h"
#include "Poco/ScopedLock.h"

int luaopen_poco_taskmanager(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::TaskManagerUserdata::TaskManager);
}

namespace LuaPoco
{

const char* POCO_TASK_MANAGER_METATABLE_NAME = "Poco.TaskManager.metatable";
const char* POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME = "Poco.TaskManagerContainer.metatable";
const char* POCO_TASK_PUBLIC_METATABLE_NAME = "Poco.Task.public.metatable";
const char* POCO_TASK_PROTECTED_METATABLE_NAME = "Poco.Task.protected.metatable";

const char* POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME = "Poco.TaskManagerContainer.lightuserdata";
const char* POCO_TASK_LUD_KEY_NAME = "Poco.Task.lightuserdata";

const char* TASK_START_FUNCTION_NAME = "onStart";
const char* TASK_CANCEL_FUNCTION_NAME = "onCancel";
const char* TASK_FAILURE_FUNCTION_NAME = "onFailure";
const char* TASK_FINISH_FUNCTION_NAME = "onFinish";
const char* TASK_PROGRESS_FUNCTION_NAME = "onProgress";
const char* TASK_CUSTOM_FUNCTION_NAME = "onCustom";

const int NOTIFICATION_BORROW_TIMEOUT_MS = 1000;

static bool getLightUserdataFromTable(lua_State* L, int tableIndex,
    const char* expected, const char* userdataName)
{
    bool result = false;
    bool tablesMatch = false;
    int top = lua_gettop(L);
    // validate the current metatable against the one we stored in the registry
    luaL_getmetatable(L, expected);
    tablesMatch = lua_getmetatable(L, tableIndex) && lua_rawequal(L, -1, -2);
    // remove any items placed on the stack by this check.
    lua_settop(L, top);

    if (tablesMatch)
    {
        lua_pushstring(L, userdataName);
        lua_rawget(L, tableIndex);
        result = lua_islightuserdata(L, -1) == 1;
    }

    // if the table did not match, also clean the stack for future calls.
    if (!result) { lua_settop(L, top); }

    return result;
}

static bool findTaskInTaskList(lua_State* L, int udIndex,
    Poco::AutoPtr<Poco::Task>& task, Poco::TaskManager& manager)
{
    bool result = false;
    Poco::Task* ludTask = static_cast<Poco::Task*>(lua_touserdata(L, udIndex));

    Poco::TaskManager::TaskList tl = manager.taskList();
    for (Poco::TaskManager::TaskList::iterator i = tl.begin(); i != tl.end(); ++i)
    {
        if (ludTask == i->get())
        {
            task = *i;
            result = true;
            break;
        }
    }

    return result;
}


Task::Task(const char* taskName) :
    Poco::Task(taskName),
    mState(NULL)
{
}

Task::~Task()
{
    lua_close(mState);
}


bool Task::prepTask(
    lua_State* L,
    int taskManagerLudIndex,
    int functionIndex,
    int firstParamIndex,
    int lastParamIndex)
{
    LuaStateHolder holder(luaL_newstate());
    // initialize the state and load poco metatables.
    luaL_openlibs(holder.state);

    if (!loadMetatables(holder.state))
    {
        lua_pushnil(L);
        lua_pushstring(L, "could not load poco library into thread's state.");
        return false;
    }

    // transfer function
    lua_pushvalue(L, functionIndex);
    bool transferred = transferValue(holder.state, L);
    lua_pop(L, 1);

    if (!transferred)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "non-copyable function at parameter %d\n", functionIndex);
        return false;
    }

    // Store the TaskManagerContainer's light userdata value in the registry such that it can
    // later be retrieved for pushNotification().
    lua_pushlightuserdata(holder.state, lua_touserdata(L, taskManagerLudIndex));
    lua_setfield(holder.state, LUA_REGISTRYINDEX, POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME);

    // setup TaskManager table, metatable with lightuserdata.
    // 0 array slots, 1 hash table slots.
    lua_createtable(holder.state, 0, 1);
    // set lightuserdata to POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME key.
    lua_pushstring(holder.state, POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME);
    lua_pushlightuserdata(holder.state, lua_touserdata(L, taskManagerLudIndex));
    lua_rawset(holder.state, -3);
    // add metatable to Task table instance.
    luaL_getmetatable(holder.state, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME);
    lua_setmetatable(holder.state, -2);

    // setup Task table, metatable, with lightuserdata.
    lua_createtable(holder.state, 0, 1);
    lua_pushstring(holder.state, POCO_TASK_LUD_KEY_NAME);
    lua_pushlightuserdata(holder.state, reinterpret_cast<void*>(this));
    lua_rawset(holder.state, -3);
    luaL_getmetatable(holder.state, POCO_TASK_PROTECTED_METATABLE_NAME);
    lua_setmetatable(holder.state, -2);

    // check if there are args present, given:
    // 1 = instance table, 2 = task name, 3 = function, 4 = start of args
    // if taskManagerLudIndex (which is at -1) is equal to 4, it means there are no args
    if (taskManagerLudIndex > 4)
    {
        for (int i = firstParamIndex; i <= lastParamIndex; ++i)
        {
            lua_pushvalue(L, i);
            transferred = transferValue(holder.state, L);
            lua_pop(L, 1);

            if (!transferred)
            {
                lua_pushnil(L);
                lua_pushfstring(L, "non-copyable value at parameter %d\n", i);
                return false;
            }
        }
    }

    // place the task's lua_State in mState.
    mState = holder.extract();
    return true;
}

void Task::runTask()
{
    // arguments are in this order:
    // 1. task function
    // 2. taskmanager object
    // 3. protected task self object
    // 4. remainder of arguments
    int result = lua_pcall(mState, lua_gettop(mState) - 1, 0, 0);

    // Poco::Task catches exceptions thrown by tasks and posts a TaskFailedNotification.
    // runTask will replicate that behavior here, instead of throwing and having Poco::Task catch.
    if (result != 0)
    {
        const char* errmsg = lua_tostring(mState, -1);
        postNotification(new Poco::TaskFailedNotification(this, Poco::Exception(errmsg, result)));
    }
}

int Task::lud_isCancelled(lua_State* L)
{
    int rv = 1;
    int isCancelled = 0;
    luaL_checktype(L, 1, LUA_TTABLE);

    if (getLightUserdataFromTable(L, 1, POCO_TASK_PUBLIC_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME) ||
        getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Poco::Task* task = reinterpret_cast<Poco::Task*>(lua_touserdata(L, -1));
        isCancelled = task->isCancelled() ? 1 : 0;
        lua_pushboolean(L, isCancelled);
    }

    return rv;
}

int Task::lud_cancel(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TTABLE);

    if (getLightUserdataFromTable(L, 1, POCO_TASK_PUBLIC_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME) ||
        getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Poco::Task* task = reinterpret_cast<Poco::Task*>(lua_touserdata(L, -1));
        task->cancel();
    }

    return rv;
}

int Task::lud_name(lua_State* L)
{
    int rv = 0;
    int isCancelled = 0;
    luaL_checktype(L, 1, LUA_TTABLE);

    if (getLightUserdataFromTable(L, 1, POCO_TASK_PUBLIC_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME) ||
        getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Poco::Task* task = reinterpret_cast<Poco::Task*>(lua_touserdata(L, -1));
        const std::string taskName = task->name();
        lua_pushlstring(L, taskName.c_str(), taskName.size());
        rv = 1;
    }

    return rv;
}

int Task::lud_progress(lua_State* L)
{
    int rv = 0;
    int isCancelled = 0;
    luaL_checktype(L, 1, LUA_TTABLE);

    if (getLightUserdataFromTable(L, 1, POCO_TASK_PUBLIC_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME) ||
        getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Poco::Task* task = reinterpret_cast<Poco::Task*>(lua_touserdata(L, -1));
        lua_Number taskProgress = task->progress();
        lua_pushnumber(L, taskProgress);
        rv = 1;
    }

    return rv;
}

int Task::lud_reset(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TTABLE);

    if (getLightUserdataFromTable(L, 1, POCO_TASK_PUBLIC_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME) ||
        getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Poco::Task* task = reinterpret_cast<Poco::Task*>(lua_touserdata(L, -1));
        task->reset();
    }

    return rv;
}

int Task::lud_state(lua_State* L)
{
    int rv = 1;
    const char* stateString = "unknown";
    luaL_checktype(L, 1, LUA_TTABLE);

    if (getLightUserdataFromTable(L, 1, POCO_TASK_PUBLIC_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME) ||
        getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Poco::Task* task = reinterpret_cast<Poco::Task*>(lua_touserdata(L, -1));
        Poco::Task::TaskState taskProgress = task->state();

        switch (taskProgress)
        {
            case Poco::Task::TASK_IDLE:
                stateString = "idle";
                break;
            case Poco::Task::TASK_STARTING:
                stateString = "starting";
                break;
            case Poco::Task::TASK_RUNNING:
                stateString = "running";
                break;
            case Poco::Task::TASK_CANCELLING:
                stateString = "cancelling";
                break;
            case Poco::Task::TASK_FINISHED:
                stateString = "finished";
                break;
            default:
                stateString = "unknown";
        }
    }

    lua_pushstring(L, stateString);
    return rv;
}

// functions to be used via a metatable on the protected interface of a Task*
int Task::lud_setState(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    const char* stateString = luaL_checkstring(L, 2);


    if (getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Poco::Task::TaskState taskState = Poco::Task::TASK_IDLE;
        Task* task = reinterpret_cast<Task*>(lua_touserdata(L, -1));

        if (std::strcmp(stateString, "idle") == 0)
            { taskState = Poco::Task::TASK_IDLE; }
        else if (std::strcmp(stateString, "starting") == 0)
            { taskState = Poco::Task::TASK_STARTING; }
        else if (std::strcmp(stateString, "running") == 0)
            { taskState = Poco::Task::TASK_RUNNING; }
        else if (std::strcmp(stateString, "cancelling") == 0)
            { taskState = Poco::Task::TASK_CANCELLING; }
        else if (std::strcmp(stateString, "finished") == 0)
            { taskState = Poco::Task::TASK_FINISHED; }
        else
            { taskState = Poco::Task::TASK_IDLE; }

        task->setState(taskState);
    }

    return rv;
}

int Task::lud_setProgress(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_Number progress = luaL_checknumber(L, 2);

    if (getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Task* task = reinterpret_cast<Task*>(lua_touserdata(L, -1));
        task->setProgress(static_cast<float>(progress));
    }

    return rv;
}

int Task::lud_sleep(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    long milliseconds = static_cast<long>(luaL_checkinteger(L, 2));

    if (getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        Task* task = reinterpret_cast<Task*>(lua_touserdata(L, -1));
        lua_pushboolean(L, task->sleep(milliseconds) ? 1 : 0);
        rv = 1;
    }

    return rv;
}

int Task::lud_postNotification(lua_State* L)
{
    int rv = 0;
    int top = lua_gettop(L);
    luaL_checktype(L, 1, LUA_TTABLE);
    
    if (getLightUserdataFromTable(L, 1, POCO_TASK_PROTECTED_METATABLE_NAME, POCO_TASK_LUD_KEY_NAME))
    {
        // get Task*
        Task* task = reinterpret_cast<Task*>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        // get TaskManagerContainer* from registry
        lua_getfield(L, LUA_REGISTRYINDEX, POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME);
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        Poco::AutoPtr<Notification> notification;
        do { notification = tmc->mPool.borrowObject(NOTIFICATION_BORROW_TIMEOUT_MS); } while (notification.isNull());
        
        for (int i = 2; i <= top; ++i)
        {
            // transferValue requires value be at the top of the stack.
            lua_pushvalue(L, i);
            if (!transferValue(notification->state, L))
            {
                lua_pushnil(L);
                lua_pushfstring(L, "non-copyable value at parameter %d.\n", i);
                tmc->mPool.returnObject(notification);
                return 2;
            }
            lua_pop(L, 1);
        }

        // Task::postNotification accepts a raw pointer
        // the AutoPtr yields with operator C* (), which does not maintain the reference count
        // when handing the pointer to the TaskManager, so a duplicate call is required.
        task->postNotification(notification.duplicate());

        lua_pushboolean(L, 1);
        rv = 1;
    }

    return rv;
}

// TaskObserver implementation
TaskObserver::TaskObserver() : mNotificationTypes(0), mState(NULL)
{
    mState = luaL_newstate();
}

TaskObserver::~TaskObserver()
{
    lua_close(mState);
}

bool TaskObserver::prepObserver(lua_State* L, int observerIndex, int taskManagerLudIndex)
{
    int top = lua_gettop(L);
    // Observer layout on stack is:
    // 1. Observer table { onStart, onFinish, some_other_data }
    // 2. TaskManager table (with associated light userdata and metatable)
    // 3. Task table (with associated light userdata and metatable)

    // check for expected notification callbacks and set flag accordingly.
    lua_getfield(L, observerIndex, TASK_START_FUNCTION_NAME);
    if (lua_isfunction(L, -1)) { mNotificationTypes |= TASK_NOTIFICATION_START; }
    lua_pop(L, 1);

    lua_getfield(L, observerIndex, TASK_CANCEL_FUNCTION_NAME);
    if (lua_isfunction(L, -1)) { mNotificationTypes |= TASK_NOTIFICATION_CANCELLED; }
    lua_pop(L, 1);

    lua_getfield(L, observerIndex, TASK_FAILURE_FUNCTION_NAME);
    if (lua_isfunction(L, -1)) { mNotificationTypes |= TASK_NOTIFICATION_FAILED; }
    lua_pop(L, 1);

    lua_getfield(L, observerIndex, TASK_FINISH_FUNCTION_NAME);
    if (lua_isfunction(L, -1)) { mNotificationTypes |= TASK_NOTIFICATION_FINISHED; }
    lua_pop(L, 1);

    lua_getfield(L, observerIndex, TASK_PROGRESS_FUNCTION_NAME);
    if (lua_isfunction(L, -1)) { mNotificationTypes |= TASK_NOTIFICATION_PROGRESS; }
    lua_pop(L, 1);

    lua_getfield(L, observerIndex, TASK_CUSTOM_FUNCTION_NAME);
    if (lua_isfunction(L, -1)) { mNotificationTypes |= TASK_NOTIFICATION_CUSTOM; }
    lua_pop(L, 1);

    if (mNotificationTypes == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, "Observer must contain at least one notification callback.");
        return false;
    }

    // prepare the observer's state with default libs and load poco.
    luaL_openlibs(mState);
    if (!loadMetatables(mState))
    {
        lua_pushnil(L);
        lua_pushstring(L, "could not load poco library into thread's state.");
        return false;
    }

    // transfer observer table to observer's internal state.
    lua_pushvalue(L, observerIndex);
    bool transferred = transferValue(mState, L);
    lua_pop(L, 1);

    if (!transferred)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "non-copyable function at parameter %d\n", observerIndex);
        return false;
    }

    // setup TaskManager table, metatable with lightuserdata.
    // 0 array slots, 1 hash table slots.
    lua_createtable(mState, 0, 1);
    // set lightuserdata to POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME key.
    lua_pushstring(mState, POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME);
    lua_pushlightuserdata(mState, lua_touserdata(L, taskManagerLudIndex));
    lua_rawset(mState, -3);
    // add metatable to Task table instance.
    luaL_getmetatable(mState, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME);
    lua_setmetatable(mState, -2);

    // setup Task table, metatable, with lightuserdata.
    lua_createtable(mState, 0, 1);
    lua_pushstring(mState, POCO_TASK_LUD_KEY_NAME);
    lua_pushlightuserdata(mState, reinterpret_cast<void*>(NULL));
    lua_rawset(mState, -3);
    luaL_getmetatable(mState, POCO_TASK_PUBLIC_METATABLE_NAME);
    lua_setmetatable(mState, -2);

    lua_settop(L, top);

    return true;
}

// TaskManagerContainer implementation
TaskManagerContainer::TaskManagerContainer(
    int minThreads,
    int maxThreads,
    int idleTimeout,
    int stackSize,
    int minPool,
    int maxPool) :
    mPool(static_cast<size_t>(minPool), static_cast<size_t>(maxPool)),
    mThreadPool(minThreads, maxThreads, idleTimeout, stackSize),
    mTaskManager(mThreadPool),
    mDestruct(0)
{
    Poco::Observer<TaskManagerContainer, Poco::TaskStartedNotification>
        taskStartedObserver(*this, &TaskManagerContainer::onTaskStarted);
    Poco::Observer<TaskManagerContainer, Poco::TaskCancelledNotification>
        taskCancelledObserver(*this, &TaskManagerContainer::onTaskCancelled);
    Poco::Observer<TaskManagerContainer, Poco::TaskFinishedNotification>
        taskFinishedObserver(*this, &TaskManagerContainer::onTaskFinished);
    Poco::Observer<TaskManagerContainer, Poco::TaskFailedNotification>
        taskFailedObserver(*this, &TaskManagerContainer::onTaskFailed);
    Poco::Observer<TaskManagerContainer, Poco::TaskProgressNotification>
        taskProgressObserver(*this, &TaskManagerContainer::onTaskProgress);
    Poco::Observer<TaskManagerContainer, Notification>
        taskCustomObserver(*this, &TaskManagerContainer::onTaskCustom);

    mTaskManager.addObserver(taskStartedObserver);
    mTaskManager.addObserver(taskCancelledObserver);
    mTaskManager.addObserver(taskFinishedObserver);
    mTaskManager.addObserver(taskFailedObserver);
    mTaskManager.addObserver(taskProgressObserver);
    mTaskManager.addObserver(taskCustomObserver);
}

TaskManagerContainer::~TaskManagerContainer()
{
    ++mDestruct;
    mTaskManager.cancelAll();
    mTaskManager.joinAll();

    Poco::Observer<TaskManagerContainer, Poco::TaskStartedNotification>
        taskStartedObserver(*this, &TaskManagerContainer::onTaskStarted);
    Poco::Observer<TaskManagerContainer, Poco::TaskCancelledNotification>
        taskCancelledObserver(*this, &TaskManagerContainer::onTaskCancelled);
    Poco::Observer<TaskManagerContainer, Poco::TaskFinishedNotification>
        taskFinishedObserver(*this, &TaskManagerContainer::onTaskFinished);
    Poco::Observer<TaskManagerContainer, Poco::TaskFailedNotification>
        taskFailedObserver(*this, &TaskManagerContainer::onTaskFailed);
    Poco::Observer<TaskManagerContainer, Poco::TaskProgressNotification>
        taskProgressObserver(*this, &TaskManagerContainer::onTaskProgress);
    Poco::Observer<TaskManagerContainer, Notification>
        taskCustomObserver(*this, &TaskManagerContainer::onTaskCustom);

    mTaskManager.removeObserver(taskStartedObserver);
    mTaskManager.removeObserver(taskCancelledObserver);
    mTaskManager.removeObserver(taskFinishedObserver);
    mTaskManager.removeObserver(taskFailedObserver);
    mTaskManager.removeObserver(taskProgressObserver);
    mTaskManager.removeObserver(taskCustomObserver);
}

void TaskManagerContainer::addObserver(Poco::AutoPtr<TaskObserver>& observer)
{
    Poco::ScopedLock<Poco::FastMutex> lock(mObserverMutex);
    mObservers.push_back(observer);
}

void TaskManagerContainer::removeObserver(TaskObserver* observer)
{
    Poco::ScopedLock<Poco::FastMutex> lock(mObserverMutex);
    for (std::vector<Poco::AutoPtr<TaskObserver> >::iterator i = mObservers.begin();
            i != mObservers.end(); ++i)
    {
        if ((*i) == observer)
        {
            mObservers.erase(i);
            break;
        }
    }
}

void TaskManagerContainer::enableTaskQueue()
{
    mQueueEnabled = 1;
}

void TaskManagerContainer::disableTaskQueue()
{
    mQueueEnabled = 0;
}

// This function expects to write the notification values into a table at the top of the stack.
int TaskManagerContainer::waitDequeueNotification(lua_State* L, long milliseconds)
{
    int rv = 0;
    int top = lua_gettop(L);
    Poco::AutoPtr<Poco::Notification> n;

    // no wait
    if (milliseconds == 0) { n = mQueue.dequeueNotification(); }
    // indefinite wait
    else if (milliseconds < 0) { n = mQueue.waitDequeueNotification(); }
    // wait up to milliseconds
    else { n = mQueue.waitDequeueNotification(milliseconds); }

    // check if a notification was obtained
    if (!n.isNull())
    {
        Poco::AutoPtr<Notification> customNotification(n.cast<Notification>());
        if (!customNotification.isNull())
        {
            int customTop = lua_gettop(customNotification->state);
            // move the Poco::Task* from the custom notification over.
            // remove the Poco::Task* from the notification's state,
            // leaving only the passed values to be transferred to the current state.
            if (lua_islightuserdata(customNotification->state, customTop))
            {
                lua_pushlightuserdata(L, lua_touserdata(customNotification->state, customTop));
                lua_setfield(L, top, "task");
                lua_pop(customNotification->state, 1);
                customTop = lua_gettop(customNotification->state);
            }

            lua_pushstring(L, "custom");
            lua_setfield(L, top, "type");

            
            int valuesTransferred = transferNotification(L, customNotification);
            // as the notification has been transferred, we are done with it, return it to the pool.
            mPool.returnObject(customNotification);

            // store the data obtained from the notification in the array part of the notification table.
            for (int valueToSet = 1; valueToSet <= valuesTransferred; ++valueToSet)
            {
                lua_pushvalue(L, top + valueToSet);
                lua_rawseti(L, top, valueToSet);
            }
            // make sure to 'nil' terminate table such that 1 to N will stop at nil.
            lua_pushnil(L);
            lua_rawseti(L, top, valuesTransferred + 1);
            // reset the stack as the notification has been stored in the provided table.
            lua_settop(L, top);
            // return true
            lua_pushboolean(L, 1);
            return 1;
        }

        // all other notification types are Poco::TaskNotification subclasses.
        // set the Poco::Task* generically for all Poco::TaskNotification subclasses.
        Poco::TaskNotification* taskNotification = dynamic_cast<Poco::TaskNotification*>(n.get());
        if (taskNotification != NULL)
        {
            lua_pushlightuserdata(L, reinterpret_cast<void*>(taskNotification->task()));
            lua_setfield(L, top, "task");
        }
        
        Poco::AutoPtr<Poco::TaskStartedNotification> startedNotification(n.cast<Poco::TaskStartedNotification>());
        if (!startedNotification.isNull())
        {
            lua_pushstring(L, "started");
            lua_setfield(L, top, "type");
            lua_pushboolean(L, 1);
            return 1;
        }

        Poco::AutoPtr<Poco::TaskCancelledNotification> cancelledNotification(n.cast<Poco::TaskCancelledNotification>());
        if (!cancelledNotification.isNull())
        {
            lua_pushstring(L, "cancelled");
            lua_setfield(L, top, "type");
            lua_pushboolean(L, 1);
            return 1;
        }

        Poco::AutoPtr<Poco::TaskFinishedNotification> finishedNotification(n.cast<Poco::TaskFinishedNotification>());
        if (!finishedNotification.isNull())
        {
            lua_pushstring(L, "finished");
            lua_setfield(L, top, "type");
            lua_pushboolean(L, 1);
            return 1;
        }

        Poco::AutoPtr<Poco::TaskFailedNotification> failedNotification(n.cast<Poco::TaskFailedNotification>());
        if (!failedNotification.isNull())
        {
            lua_pushstring(L, "failed");
            lua_setfield(L, top, "type");

            const Poco::Exception& e = failedNotification->reason();
            lua_pushinteger(L, e.code());
            lua_setfield(L, top, "code");
            
            lua_pushlstring(L, e.displayText().c_str(), e.displayText().size());
            lua_setfield(L, top, "message");
            
            lua_pushboolean(L, 1);
            return 1;
        }

        Poco::AutoPtr<Poco::TaskProgressNotification> progressNotification(n.cast<Poco::TaskProgressNotification>());
        if (!progressNotification.isNull())
        {
            lua_pushstring(L, "progress");
            lua_setfield(L, top, "type");

            lua_pushnumber(L, static_cast<lua_Number>(progressNotification->progress()));
            lua_setfield(L, top, "progress");
            
            lua_pushboolean(L, 1);
            return 1;
        }
    }
    // else no notification was obtained, return nothing.

    return rv;
}

int TaskManagerContainer::lud_count(lua_State* L)
{
    int rv = 0;

    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));
        lua_pushinteger(L, tmc->mTaskManager.count());
        rv = 1;
    }

    return rv;
}

int TaskManagerContainer::lud_taskList(lua_State* L)
{
    int rv = 0;

    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));

        Poco::TaskManager::TaskList tl = tmc->mTaskManager.taskList();
        lua_createtable(L, static_cast<int>(tl.size()), 0);

        int tableIndex = 1;
        for (Poco::TaskManager::TaskList::iterator i = tl.begin(); i != tl.end(); ++i)
        {
            lua_pushlightuserdata(L, reinterpret_cast<void*>(i->get()));
            lua_rawseti(L, -2, tableIndex++);
        }

        rv = 1;
    }

    return rv;
}

int TaskManagerContainer::lud_cancelAll(lua_State* L)
{
    int rv = 0;

    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));
        tmc->mTaskManager.cancelAll();
    }

    return rv;
}

int TaskManagerContainer::lud_start(lua_State* L)
{
    int rv = 0;
    int lastParamIndex = lua_gettop(L);
    const char* taskName = luaL_checkstring(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));

        if (tmc->mDestruct > 0)
        {
            lua_pushnil(L);
            lua_pushstring(L, "TaskManager is destructing, no tasks can be started.");
            return 2;
        }

        // use 'shared' Poco::AutoPtr which means the refcount gets a bump to 2 right away.
        // even though the TaskManager 'takes ownership', the refcount is already bumped.
        // this enables us to properly delete the Task* if start() fails, otherwise, it decrements the
        // refcount back to 1 with the TaskManager owning it.
        Poco::AutoPtr<Task> newTask(new Task(taskName), true);

        try
        {
            // TaskManagerContainer*, lua_Function, start Param, endParam
            if (newTask->prepTask(L, lua_gettop(L), 3, 4, lastParamIndex))
            {
                tmc->mTaskManager.start(newTask);
                Poco::Task* baseTaskPtr = newTask;
                lua_pushlightuserdata(L, static_cast<void*>(baseTaskPtr));
                rv = 1;
            }
            else // prep task on failure returns 2 values:  nil, "errmsg"
                rv = 2;
        }
        catch (const Poco::Exception& e)
        {
            rv = pushPocoException(L, e);
        }
        catch (...)
        {
            rv = pushUnknownException(L);
        }
    }
    else
        rv = luaL_error(L, "invalid argument #2, expected lightuserdata.");

    return rv;
}
int TaskManagerContainer::lud_addObserver(lua_State* L)
{
    int rv = 0;
    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        luaL_checktype(L, 2, LUA_TTABLE);
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));
        
        try
        {
            Poco::AutoPtr<TaskObserver> observer(new TaskObserver);
            
            if (observer->prepObserver(L, 2, lua_gettop(L)))
            {
                tmc->addObserver(observer);
                rv = 1;
            }
            else // prep task on failure returns 2 values:  nil, "errmsg"
                rv = 2;
    
        }
        catch (const Poco::Exception& e)
        {
            rv = pushPocoException(L, e);
        }
        catch (...)
        {
            rv = pushUnknownException(L);
        }
    }

    return rv;
}

int TaskManagerContainer::lud_removeObserver(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TLIGHTUSERDATA);
    
    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));
        TaskObserver* observer = reinterpret_cast<TaskObserver*>(lua_touserdata(L, 2));
        
        tmc->removeObserver(observer);
    }

    return 0;
}

int TaskManagerContainer::lud_enableTaskQueue(lua_State* L)
{
    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));
        tmc->enableTaskQueue();
    }

    return 0;
}

int TaskManagerContainer::lud_disableTaskQueue(lua_State* L)
{
    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));
        tmc->disableTaskQueue();
    }

    return 0;
}

int TaskManagerContainer::lud_dequeueNotification(lua_State* L)
{
    int rv = 0;
    int top = lua_gettop(L);
    
    luaL_checktype(L, 2, LUA_TTABLE);
    
    if (getLightUserdataFromTable(L, 1, POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME,
        POCO_TASK_MANAGER_CONTAINER_LUD_KEY_NAME))
    {
        TaskManagerContainer* tmc = reinterpret_cast<TaskManagerContainer*>(lua_touserdata(L, -1));
        lua_Integer waitMs = (top > 2) && lua_isnumber(L, 3) ? lua_tointeger(L, 3) : 0;
        lua_pushvalue(L, 2);
        rv = tmc->waitDequeueNotification(L, static_cast<long>(waitMs));
        
    }

    return rv;
}

void TaskManagerContainer::getObserversOfType(
        std::vector<Poco::AutoPtr<TaskObserver> >& observersToNotify,
        int taskNotificationType)
{
    Poco::ScopedLock<Poco::FastMutex> lock(mObserverMutex);    
    for (std::vector<Poco::AutoPtr<TaskObserver> >::iterator observer = mObservers.begin();
        observer != mObservers.end(); ++observer)
    {
        if ((*observer)->mNotificationTypes & taskNotificationType)
        {
            observersToNotify.push_back(*observer);
        }
    }
}

bool TaskManagerContainer::notifyObserver(
        Poco::AutoPtr<TaskObserver>& observer,
        Poco::Task* task,
        const char* callbackName)
{
    bool result = false;

    // LuaPoco's use of lua_States is to enable message passing, so lock all multithreaded access
    // to individual states.
    Poco::ScopedLock<Poco::FastMutex> lock(observer->mMutex);
    int top = lua_gettop(observer->mState);

    // replace the light userdata value in the Task table.
    lua_pushlightuserdata(observer->mState, reinterpret_cast<void*>(task));
    lua_setfield(observer->mState, 3, POCO_TASK_LUD_KEY_NAME);

    // fetch the desired callback.
    lua_getfield(observer->mState, 1, callbackName);
    if (lua_isfunction(observer->mState, -1))
    {
        // Observer table
        lua_pushvalue(observer->mState, 1);
        // TaskManager table
        lua_pushvalue(observer->mState, 2);
        // Task table
        lua_pushvalue(observer->mState, 3);
        // extra parameters to callback
        for (int extraValue = 4; extraValue <= top; ++extraValue)
        {
            lua_pushvalue(observer->mState, extraValue);
        }

        // 3 base arguments + extra params, expecting 0 return values, no error function.
        // determine what to do on an error condition for observers.
        int observerResult = lua_pcall(observer->mState, top, 0, 0);
        if (observerResult == 0)
        {
            result = true;
        }
        else
        {
            // in the advent of a failure, the task will no longer accept notifications as the
            // state is in an unknown condition except for the error message and code, which will
            // be retrieved later.
            observer->mNotificationTypes = 0;
            // stack will stay in the state:
            // -1 code
            // -2 error message
            lua_pushinteger(observer->mState, observerResult);
            return false;
        }
    }

    // reset observer's stack back to 3 values on stack
    // 1. Observer table
    // 2. TaskManager table
    // 3. Task table
    lua_settop(observer->mState, 3);

    return result;
}


// Poco::Observer calls Poco::Notification::duplicate() prior to calling the callback
// As such, it is safe to permit the AutoPtr here to assume "ownership" of the notification.
void TaskManagerContainer::onTaskStarted(Poco::TaskStartedNotification* sn)
{
    Poco::AutoPtr<Poco::TaskStartedNotification> tsn(sn);

    if (mQueueEnabled) { mQueue.enqueueNotification(tsn); }
    
    std::vector<Poco::AutoPtr<TaskObserver> > observersToNotify;
    getObserversOfType(observersToNotify, TASK_NOTIFICATION_START);

    for (std::vector<Poco::AutoPtr<TaskObserver> >::iterator observer = observersToNotify.begin();
        observer != observersToNotify.end(); ++observer)
    {
        notifyObserver(*observer, tsn->task(), TASK_START_FUNCTION_NAME);
    }
}

void TaskManagerContainer::onTaskCancelled(Poco::TaskCancelledNotification* cn)
{
    Poco::AutoPtr<Poco::TaskCancelledNotification> tcn(cn);
    
    if (mQueueEnabled) { mQueue.enqueueNotification(tcn); }
    
    std::vector<Poco::AutoPtr<TaskObserver> > observersToNotify;
    getObserversOfType(observersToNotify, TASK_NOTIFICATION_CANCELLED);

    for (std::vector<Poco::AutoPtr<TaskObserver> >::iterator observer = observersToNotify.begin();
        observer != observersToNotify.end(); ++observer)
    {
        notifyObserver(*observer, tcn->task(), TASK_CANCEL_FUNCTION_NAME);
    }
}

void TaskManagerContainer::onTaskFinished(Poco::TaskFinishedNotification* fn)
{
    Poco::AutoPtr<Poco::TaskFinishedNotification> tfn(fn);
    
    if (mQueueEnabled) { mQueue.enqueueNotification(tfn); }
    
    std::vector<Poco::AutoPtr<TaskObserver> > observersToNotify;
    getObserversOfType(observersToNotify, TASK_NOTIFICATION_FINISHED);

    for (std::vector<Poco::AutoPtr<TaskObserver> >::iterator observer = observersToNotify.begin();
        observer != observersToNotify.end(); ++observer)
    {
        notifyObserver(*observer, tfn->task(), TASK_FINISH_FUNCTION_NAME);
    }
}

void TaskManagerContainer::onTaskFailed(Poco::TaskFailedNotification* fn)
{
    Poco::AutoPtr<Poco::TaskFailedNotification> tfn(fn);
    
    if (mQueueEnabled) { mQueue.enqueueNotification(tfn); }
    
    std::vector<Poco::AutoPtr<TaskObserver> > observersToNotify;
    getObserversOfType(observersToNotify, TASK_NOTIFICATION_FAILED);

    for (std::vector<Poco::AutoPtr<TaskObserver> >::iterator observer = observersToNotify.begin();
        observer != observersToNotify.end(); ++observer)
    {
        const Poco::Exception& e = tfn->reason();
        lua_pushinteger((*observer)->mState, e.code());
        lua_pushlstring((*observer)->mState, e.displayText().c_str(), e.displayText().size());
        notifyObserver(*observer, tfn->task(), TASK_FAILURE_FUNCTION_NAME);
    }
}

void TaskManagerContainer::onTaskProgress(Poco::TaskProgressNotification* pn)
{
    Poco::AutoPtr<Poco::TaskProgressNotification> tpn(pn);
    
    if (mQueueEnabled) { mQueue.enqueueNotification(tpn); }
    
    std::vector<Poco::AutoPtr<TaskObserver> > observersToNotify;
    getObserversOfType(observersToNotify, TASK_NOTIFICATION_PROGRESS);

    for (std::vector<Poco::AutoPtr<TaskObserver> >::iterator observer = observersToNotify.begin();
        observer != observersToNotify.end(); ++observer)
    {
        lua_pushnumber((*observer)->mState, static_cast<lua_Number>(tpn->progress()));
        notifyObserver(*observer, tpn->task(), TASK_PROGRESS_FUNCTION_NAME);
    }
}

void TaskManagerContainer::onTaskCustom(Notification* n)
{
    Poco::AutoPtr<Notification> cn(n);
    
    if (mQueueEnabled)
    {
        // obtain a Notification from the ObjectPool and copy this one to it.  this allows:
        // 1. this notification to be returned to the pool after this function is done.
        // 2. NotificationQueue can safely modify the copied lua_State as needed, and also return
        //  it to the pool when done.
        Poco::AutoPtr<Notification> cnDupe;
        do { cnDupe = mPool.borrowObject(NOTIFICATION_BORROW_TIMEOUT_MS); } while (cnDupe.isNull());
        transferNotification(cnDupe->state, cn);
        mQueue.enqueueNotification(cnDupe);
    }
    
    int customTop = lua_gettop(cn->state);
    Poco::Task* task = static_cast<Poco::Task*>(lua_touserdata(cn->state, customTop));
    // remove the Poco::Task* from the stack to prepare for transfer.
    lua_pop(cn->state, 1);
    customTop = lua_gettop(cn->state);

    std::vector<Poco::AutoPtr<TaskObserver> > observersToNotify;
    getObserversOfType(observersToNotify, TASK_NOTIFICATION_CUSTOM);

    for (std::vector<Poco::AutoPtr<TaskObserver> >::iterator observer = observersToNotify.begin();
        observer != observersToNotify.end(); ++observer)
    {
        transferNotification((*observer)->mState, cn);
        notifyObserver(*observer, task, TASK_CUSTOM_FUNCTION_NAME);
    }

    mPool.returnObject(cn);
}


// TaskManagerUserdata implementation
TaskManagerUserdata::TaskManagerUserdata(
    int minThreads,
    int maxThreads,
    int idleTimeout,
    int stackSize,
    int minPool,
    int maxPool)
    : mContainer(new TaskManagerContainer(minThreads, maxThreads, idleTimeout, stackSize, minPool, maxPool))
{
}

// construct new userdata from existing SharedPtr
TaskManagerUserdata::TaskManagerUserdata(Poco::SharedPtr<TaskManagerContainer>& tmc) :
    mContainer(tmc)
{
}

TaskManagerUserdata::~TaskManagerUserdata()
{
}

bool TaskManagerUserdata::copyToState(lua_State *L)
{
    TaskManagerUserdata* tmud = new(lua_newuserdata(L, sizeof *tmud)) TaskManagerUserdata(mContainer);
    setupPocoUserdata(L, tmud, POCO_TASK_MANAGER_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool TaskManagerUserdata::registerTaskManager(lua_State* L)
{
    struct UserdataMethod taskPublicMethods[] =
    {
        { "isCancelled", Task::lud_isCancelled },
        { "cancel", Task::lud_cancel },
        { "name", Task::lud_name },
        { "progress", Task::lud_progress },
        { "reset", Task::lud_reset },
        { "state", Task::lud_state },
        { NULL, NULL}
    };

    struct UserdataMethod taskProtectedMethods[] =
    {
        // copy of public interface
        { "isCancelled", Task::lud_isCancelled },
        { "cancel", Task::lud_cancel },
        { "name", Task::lud_name },
        { "progress", Task::lud_progress },
        { "reset", Task::lud_reset },
        { "state", Task::lud_state },
        // protected interface
        { "setState", Task::lud_setState },
        { "setProgress", Task::lud_setProgress },
        { "sleep", Task::lud_sleep },
        { "postNotification", Task::lud_postNotification },
        { NULL, NULL}
    };

    struct UserdataMethod taskManagerContainerMethods[] =
    {
        { "count", TaskManagerContainer::lud_count },
        { "taskList", TaskManagerContainer::lud_taskList },
        { "cancelAll", TaskManagerContainer::lud_cancelAll },
        { "start", TaskManagerContainer::lud_start },
        { "addObserver", TaskManagerContainer::lud_addObserver },
        { "removeObserver", TaskManagerContainer::lud_removeObserver },
        { "enableTaskQueue", TaskManagerContainer::lud_enableTaskQueue },
        { "disableTaskQueue", TaskManagerContainer::lud_disableTaskQueue },
        { "dequeueNotification", TaskManagerContainer::lud_dequeueNotification },
        { NULL, NULL }
    };

    struct UserdataMethod taskManagerMethods[] =
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "count", count },
        { "taskList", taskList },
        { "cancelAll", cancelAll },
        { "joinAll", joinAll },
        { "start", start },
        { "addObserver", addObserver },
        { "removeObserver", removeObserver },
        { "enableTaskQueue", enableTaskQueue },
        { "disableTaskQueue", disableTaskQueue },
        { "dequeueNotification", dequeueNotification },

        { "isTaskCancelled", isTaskCancelled },
        { "taskCancel", taskCancel },
        { "taskName", taskName },
        { "taskProgress", taskProgress },
        { "taskReset", taskReset },
        { "taskState", taskState },
        { NULL, NULL}
    };

    setupUserdataMetatable(L,
        POCO_TASK_PUBLIC_METATABLE_NAME, taskPublicMethods);
    setupUserdataMetatable(L,
        POCO_TASK_PROTECTED_METATABLE_NAME, taskProtectedMethods);
    setupUserdataMetatable(L,
        POCO_TASK_MANAGER_CONTAINER_METATABLE_NAME, taskManagerContainerMethods);
    setupUserdataMetatable(L,
        POCO_TASK_MANAGER_METATABLE_NAME, taskManagerMethods);

    return true;
}

/// create a new taskmanager userdata.
// @param[opt] TaskManagerSettings table
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see TaskManagerSettings
int TaskManagerUserdata::TaskManager(lua_State* L)
{
    int rv = 0;
    // defaults
    int minThreads = 1;
    int maxThreads = 16;
    int idleTime = 60;
    int stackSize = 0;
    int minNotificationPool = 8;
    int maxNotificationPool = 16;

    int firstArg = lua_istable(L, 1) ? 2 : 1;
    int top = lua_gettop(L);

    if (top > firstArg)
    {
        luaL_checktype(L, 2, LUA_TTABLE);
        
        lua_getfield(L, firstArg, "minThreads");
        if (!lua_isnil(L, -1)) { minThreads = static_cast<int>(lua_tointeger(L, -1)); }
        lua_getfield(L, firstArg, "maxThreads");
        if (!lua_isnil(L, -1)) { maxThreads = static_cast<int>(lua_tointeger(L, -1)); }
        lua_getfield(L, firstArg, "idleTime");
        if (!lua_isnil(L, -1)) { idleTime = static_cast<int>(lua_tointeger(L, -1)); }
        lua_getfield(L, firstArg, "stackSize");
        if (!lua_isnil(L, -1)) { stackSize = static_cast<int>(lua_tointeger(L, -1)); }
        lua_getfield(L, firstArg, "minNotificationPool");
        if (!lua_isnil(L, -1)) { minNotificationPool = static_cast<int>(lua_tointeger(L, -1)); }
        lua_getfield(L, firstArg, "maxNotificationPool");
        if (!lua_isnil(L, -1)) { maxNotificationPool = static_cast<int>(lua_tointeger(L, -1)); }
    }

    try
    {
        TaskManagerUserdata* tmud = new(lua_newuserdata(L, sizeof *tmud))
            TaskManagerUserdata(minThreads, maxThreads, idleTime, stackSize,
                                minNotificationPool, maxNotificationPool);
        setupPocoUserdata(L, tmud, POCO_TASK_MANAGER_METATABLE_NAME);
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

// metamethod infrastructure
int TaskManagerUserdata::metamethod__tostring(lua_State* L)
{
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    lua_pushfstring(L, "Poco.TaskManager (%p)", static_cast<void*>(tmud));
    return 1;
}

// userdata methods

///
// @type taskmanager

/// Dequeue a notification from the notificationqueue.
// This function does not block.
// @int[opt] timeout timeout value in milliseconds to block waiting for notification.
// if parameter is not supplied, waitDequeue will block indefinitely waiting for a notification.
// @return nil or string (notification type)
// @return ... one or more values that were supplied to enqueue.
// @function waitDequeue

/// Get the count of tasks currently running on the TaskManager.
// @return count of currently running tasks.
// @function count
int TaskManagerUserdata::count(lua_State* L)
{
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);
    lua_pushinteger(L, tmud->mContainer->mTaskManager.count());
    return 1;
}

/// Get a copied list of the tasks currently running on the TaskManager.
// @return array of currently running tasks containing lightuserdata values.
// @function taskList
int TaskManagerUserdata::taskList(lua_State* L)
{
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    Poco::TaskManager::TaskList tl = tmud->mContainer->mTaskManager.taskList();
    lua_createtable(L, static_cast<int>(tl.size()), 0);

    int tableIndex = 1;
    for (Poco::TaskManager::TaskList::iterator i = tl.begin(); i != tl.end(); ++i)
    {
        lua_pushlightuserdata(L, reinterpret_cast<void*>(i->get()));
        lua_rawseti(L, -2, tableIndex++);
    }

    return 1;
}

/// Sets a flag for all tasks running on the TaskManager indicating that they should stop.
// Tasks will see the flag is set when inspecting the return value Task:isCancelled() or Task:sleep()
// @function cancelAll
int TaskManagerUserdata::cancelAll(lua_State* L)
{
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);
    tmud->mContainer->mTaskManager.cancelAll();

    return 0;
}

/// Waits for all tasks currently running on the TaskManager to exit.
// Note the threads running on the underlying thread pool do not join until the TaskManager is garbage collected.
// @return boolean indicating if there were any errors encountered any TaskObservers during operation.
// @function joinAll
int TaskManagerUserdata::joinAll(lua_State* L)
{
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);
    tmud->mContainer->mTaskManager.joinAll();

    return 0;
}

/// Start a Lua function as a Task on the TaskManager.
// @param taskStart a function to be copied to the new state.  Upvalues are ignored.
// @param ... A variable list of basic Lua types or poco userdata that are noted to be copyable/sharable between threads.
// @return true or nil. (error)
// @return error message.
// @function start
int TaskManagerUserdata::start(lua_State* L)
{
    int rv = 0;
    int lastParamIndex = lua_gettop(L);
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    const char* taskName = luaL_checkstring(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    if (tmud->mContainer->mDestruct > 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, "TaskManager is destructing, no tasks can be started.");
        return 2;
    }

    // use 'shared' Poco::AutoPtr which means the refcount gets a bump to 2 right away.
    // even though the TaskManager 'takes ownership', the refcount is already bumped.
    // this enables us to properly delete the Task* if start() fails, otherwise, it decrements the
    // refcount back to 1 with the TaskManager owning it.
    Poco::AutoPtr<Task> newTask(new Task(taskName), true);

    try
    {
        lua_pushlightuserdata(L, reinterpret_cast<void*>(tmud->mContainer.get()));
        if (newTask->prepTask(L, lua_gettop(L), 3, 4, lastParamIndex))
        {
            tmud->mContainer->mTaskManager.start(newTask);
            Poco::Task* baseTaskPtr = newTask;
            lua_pushlightuserdata(L, static_cast<void*>(baseTaskPtr));
            rv = 1;
        }
        else // prep task on failure returns 2 values:  nil, "errmsg"
            rv = 2;

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

/// Adds an Observer table of callbacks that receive notifications from the TaskManager.
// The observer functions are copied to their own Lua state, and are invoked on the TaskManager's threadpool.
// @param observer table containing Lua Functions that are called when Notifications are received from the TaskManager.
// @return observer_lightuserdata or nil. (error)
// @return error message.
// @function addObserver
int TaskManagerUserdata::addObserver(lua_State* L)
{
    int rv = 0;
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_pushlightuserdata(L, reinterpret_cast<void*>(tmud->mContainer.get()));
    
    try
    {
        Poco::AutoPtr<TaskObserver> observer(new TaskObserver);
        
        if (observer->prepObserver(L, 2, 3))
        {
            tmud->mContainer->addObserver(observer);
            rv = 1;
        }
        else // prep task on failure returns 2 values:  nil, "errmsg"
            rv = 2;

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

/// Removes an observer from the TaskManager
// The observer functions are copied to their own Lua state, and are invoked on the TaskManager's threadpool.
// @param observer_lightuserdata value returned from addObserver.
// @function removeObserver
int TaskManagerUserdata::removeObserver(lua_State* L)
{
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);
    luaL_checktype(L, 2, LUA_TLIGHTUSERDATA);

    TaskObserver* observer = static_cast<TaskObserver*>(lua_touserdata(L, 2));
    
    tmud->mContainer->removeObserver(observer);
    return 0;
}

/// Enables receiving Task notifications via a queue inside the TaskManager.
// This should be turned on before adding any tasks to the queue, otherwise there is potential for notifications to be missed.
// @function enableTaskQueue
int TaskManagerUserdata::enableTaskQueue(lua_State* L)
{
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);
    tmud->mContainer->enableTaskQueue();

    return 0;
}

/// Enables receiving Task notifications via a queue inside the TaskManager.
// This should be turned on before adding any tasks to the queue, otherwise there is potential for notifications to be missed.
// @function disableTaskQueue
int TaskManagerUserdata::disableTaskQueue(lua_State* L)
{
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);
    tmud->mContainer->disableTaskQueue();

    return 0;
}

/// Dequeues a notification from the TaskManager
// @param notification table that receives notification values.
// @function disableTaskQueue
int TaskManagerUserdata::dequeueNotification(lua_State* L)
{
    int top = lua_gettop(L);
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);    
    luaL_checktype(L, 2, LUA_TTABLE);

    lua_Integer waitMs = (top > 2) && lua_isnumber(L, 3) ? lua_tointeger(L, 3) : 0;
    lua_pushvalue(L, 2);
    
    return tmud->mContainer->waitDequeueNotification(L, static_cast<long>(waitMs));
}

/// Returns if a particular Task is cancelled or not.
// @param task_lightuserdata value returned by start, or found by taskList
// @function isTaskCancelled
// @return boolean indicating if the task is running or not.
int TaskManagerUserdata::isTaskCancelled(lua_State* L)
{
    int rv = 1;
    int isTaskCancelled = 1;

    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    if (lua_islightuserdata(L, 2))
    {
        Poco::AutoPtr<Poco::Task> task;
        if (findTaskInTaskList(L, 2, task, tmud->mContainer->mTaskManager))
        {
            isTaskCancelled = task->isCancelled() ? 1 : 0;
        }

        lua_pushboolean(L, isTaskCancelled);
    }
    else
        rv = luaL_error(L, "invalid argument #2, expected lightuserdata.");

    return rv;
}

/// Request cancellation of a Task.
// @param task_lightuserdata value returned by start, or found by taskList
// @function taskCancel
int TaskManagerUserdata::taskCancel(lua_State* L)
{
    int rv = 0;

    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    if (lua_islightuserdata(L, 2))
    {
        Poco::AutoPtr<Poco::Task> task;
        if (findTaskInTaskList(L, 2, task, tmud->mContainer->mTaskManager))
        {
            task->cancel();
        }
    }
    else
        rv = luaL_error(L, "invalid argument #2, expected lightuserdata.");

    return rv;
}

/// Returns the name of a Task.
// @param task_lightuserdata value returned by start, or found by taskList
// @function taskName
// @return string name of the task.
int TaskManagerUserdata::taskName(lua_State* L)
{
    int rv = 0;

    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    if (lua_islightuserdata(L, 2))
    {
        Poco::AutoPtr<Poco::Task> task;
        if (findTaskInTaskList(L, 2, task, tmud->mContainer->mTaskManager))
        {
            const std::string& taskName = task->name();
            lua_pushlstring(L, taskName.c_str(), taskName.size());
            rv = 1;
        }
    }
    else
        rv = luaL_error(L, "invalid argument #2, expected lightuserdata.");

    return rv;
}

/// Returns the progress of a Task.
// @param task_lightuserdata value returned by start, or found by taskList
// @function taskProgress
// @return number indicating the percentage progress of the task from 0.0 to 100.0
int TaskManagerUserdata::taskProgress(lua_State* L)
{
    int rv = 0;

    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    if (lua_islightuserdata(L, 2))
    {
        Poco::AutoPtr<Poco::Task> task;
        if (findTaskInTaskList(L, 2, task, tmud->mContainer->mTaskManager))
        {
            lua_Number taskProgress = task->progress();
            lua_pushnumber(L, taskProgress);
            rv = 1;
        }
    }
    else
        rv = luaL_error(L, "invalid argument #2, expected lightuserdata.");

    return rv;
}

/// Resets the progress and cancellation status of the Task.
// @param task_lightuserdata value returned by start, or found by taskList
// @function taskReset
// @return number indicating the percentage progress of the task from 0.0 to 100.0
int TaskManagerUserdata::taskReset(lua_State* L)
{
    int rv = 0;

    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    if (lua_islightuserdata(L, 2))
    {
        Poco::AutoPtr<Poco::Task> task;
        if (findTaskInTaskList(L, 2, task, tmud->mContainer->mTaskManager))
        {
            task->reset();
        }
    }
    else
        rv = luaL_error(L, "invalid argument #2, expected lightuserdata.");

    return rv;
}

/// Returns the current state of the Task.
// @param task_lightuserdata value returned by start, or found by taskList
// @function taskState
// @return string indicating the current state of the Task.
int TaskManagerUserdata::taskState(lua_State* L)
{
    int rv = 1;
    const char* stateString = "unknown";
    TaskManagerUserdata* tmud = checkPrivateUserdata<TaskManagerUserdata>(L, 1);

    if (lua_islightuserdata(L, 2))
    {
        Poco::AutoPtr<Poco::Task> task;
        if (findTaskInTaskList(L, 2, task, tmud->mContainer->mTaskManager))
        {
            Poco::Task::TaskState taskProgress = task->state();
            switch (taskProgress)
            {
                case Poco::Task::TASK_IDLE:
                    stateString = "idle";
                    break;
                case Poco::Task::TASK_STARTING:
                    stateString = "starting";
                    break;
                case Poco::Task::TASK_RUNNING:
                    stateString = "running";
                    break;
                case Poco::Task::TASK_CANCELLING:
                    stateString = "cancelling";
                    break;
                case Poco::Task::TASK_FINISHED:
                    stateString = "finished";
                    break;
                default:
                    stateString = "unknown";
            }
        }
    }
    else
        rv = luaL_error(L, "invalid argument #2, expected lightuserdata.");

    lua_pushstring(L, stateString);
    return rv;
}

} // LuaPoco
