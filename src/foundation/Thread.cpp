/// A platform-independent wrapper to an operating system thread.
// Every thread userdata gets a unique (within its process) numeric thread ID. Furthermore, a thread can be assigned a name. The name of a thread can be changed at any time. 
//
// Note: Threads are implemented using separate Lua states.  
// Lua code is run in the thread by dumping a function from one state, and loading it into the thread's state. Upvalues are not dumped.
// Basic Lua types like booleans, numbers, strings, and tables can be copied to the new thread.
// Userdata from the poco module that are noted to be copyable/sharable are also able to be passed to a new thread.
//
// Note: Synchronization mechanisms like fastmutex, mutex, and semaphore can be used to communicate, but IPC mechanisms that avoid locking complications like pipes, sockets, and notifications are recommended instead.
// @module thread

#include "Thread.h"
#include "StateTransfer.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_thread(lua_State* L)
{
    LuaPoco::ThreadUserdata::registerThread(L);
    return LuaPoco::loadConstructor(L, LuaPoco::ThreadUserdata::Thread);
}

namespace LuaPoco
{

const char* POCO_THREAD_METATABLE_NAME = "Poco.Thread.metatable";

ThreadUserdata::ThreadUserdata() :
    mThread(), mJoined(false), mStarted(false), mThreadState(NULL), mParamCount(0),
    mThreadResult(0)
{
}

ThreadUserdata::~ThreadUserdata()
{
    if (mStarted && !mJoined)
        mThread.join();
    if (mThreadState)
        lua_close(mThreadState);
}

// register metatable for this class
bool ThreadUserdata::registerThread(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "name", name },
        { "id", id },
        { "isRunning", isRunning },
        { "join", join },
        { "stackSize", stackSize },
        { "start", start },
        { "priority", priority },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_THREAD_METATABLE_NAME, methods);
    return true;
}
/// constructs a new thread userdata.
// @string[opt] priority a string indicating thread priority, "lowest", "low", "normal", "high", "highest", default is "normal".
// @string[opt] name the name of the thread, default is no name.
// @int[opt] stackSize the size of the stack for the thread, default is platform defined.
// @return thread userdata or nil. (error)
// @function new
int ThreadUserdata::Thread(lua_State* L)
{
    int rv = 0;
    int top = lua_gettop(L);
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    
    const char* priority = NULL;
    const char* name = NULL;
    lua_Integer stackSize = 0;
    
    if (top >= firstArg)
        priority = luaL_checkstring(L, firstArg);
    if (top > firstArg)
        name = luaL_checkstring(L, firstArg + 1);
    if (top > firstArg + 1)
        stackSize = luaL_checkinteger(L, firstArg + 2);

    try
    {
        ThreadUserdata* thud = new(lua_newuserdata(L, sizeof *thud)) ThreadUserdata();
        setupPocoUserdata(L, thud, POCO_THREAD_METATABLE_NAME);
        rv = 1;
        
        Poco::Thread::Priority p = Poco::Thread::PRIO_NORMAL;
        if (top > 0 && priority)
        {
            if (strcmp(priority, "lowest") == 0)
                p = Poco::Thread::PRIO_LOWEST;
            else if (strcmp(priority, "low") == 0)
                p = Poco::Thread::PRIO_LOW;
            else if (strcmp(priority, "normal") == 0)
                p = Poco::Thread::PRIO_NORMAL;
            else if (strcmp(priority, "high") == 0)
                p = Poco::Thread::PRIO_HIGH;
            else if (strcmp(priority, "highest") == 0)
                p = Poco::Thread::PRIO_HIGH;
            else
            {
                lua_pushnil(L);
                lua_pushfstring(L, "invalid priority value: %s", priority);
                return 2;
            }
        }
        if (top > 0)
            thud->mThread.setPriority(p);
        if (top > 1 && name)
            thud->mThread.setName(name);
        
        if (top > 2 && stackSize)
            thud->mThread.setStackSize(stackSize);
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
// @type thread
int ThreadUserdata::metamethod__tostring(lua_State* L)
{
    ThreadUserdata* thud = checkPrivateUserdata<ThreadUserdata>(L, 1);
    lua_pushfstring(L, "Poco.Thread (%p)", static_cast<void*>(thud));
    return 1;
}

// userdata methods

/// Get or set the thread's name.
// Pass no value to get the thread name.
// @string name if name is passed, the thread's name is set, otherwise the current name is returned.
// @return name as a string.
// @function name
int ThreadUserdata::name(lua_State* L)
{
    int rv = 0;
    ThreadUserdata* thud = checkPrivateUserdata<ThreadUserdata>(L, 1);
    
    const char* name = NULL;
    int top = lua_gettop(L);
    
    if (top > 1)
        name = luaL_checkstring(L, 2);
    
    try
    {
        if (top > 1)
        {
            thud->mThread.setName(name);
            lua_pushboolean(L, 1);
            rv = 1;
        }
        else
        {
            name = thud->mThread.getName().c_str();
            lua_pushstring(L, name);
            rv = 1;
        }
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Get or set the thread's priority.
// Pass no value to get the thread's priority.
// @string[opt] priority if priority is passed "lowest", "low", "normal", "high", or "highest", the priority will be set, otherwise the current priority is returned.
// @return priority as a string.
// @function priority
int ThreadUserdata::priority(lua_State* L)
{
    int rv = 0;
    ThreadUserdata* thud = checkPrivateUserdata<ThreadUserdata>(L, 1);
    
    const char* priority = NULL;
    int top = lua_gettop(L);
    
    if (top > 1)
        priority = luaL_checkstring(L, 2);
    
    try
    {
        Poco::Thread::Priority p;
        
        if (top > 1)
        {
            Poco::Thread::Priority p = Poco::Thread::PRIO_NORMAL;
            if (strcmp(priority, "lowest") == 0)
                p = Poco::Thread::PRIO_LOWEST;
            else if (strcmp(priority, "low") == 0)
                p = Poco::Thread::PRIO_LOW;
            else if (strcmp(priority, "normal") == 0)
                p = Poco::Thread::PRIO_NORMAL;
            else if (strcmp(priority, "high") == 0)
                p = Poco::Thread::PRIO_HIGH;
            else if (strcmp(priority, "highest") == 0)
                p = Poco::Thread::PRIO_HIGH;
            else
            {
                lua_pushnil(L);
                lua_pushfstring(L, "invalid priority value: %s", priority);
                return 2;
            }
            
            thud->mThread.setPriority(p);
            lua_pushboolean(L, 1);
            rv = 1;
        }
        else
        {
            const char* priorityStr = "normal";
            
            p = thud->mThread.getPriority();
            if (p == Poco::Thread::PRIO_LOWEST)
                priorityStr = "lowest";
            else if (p == Poco::Thread::PRIO_LOW)
                priorityStr = "low";
            else if (p == Poco::Thread::PRIO_NORMAL)
                priorityStr = "normal";
            else if (p == Poco::Thread::PRIO_HIGH)
                priorityStr = "high";
            else if (p == Poco::Thread::PRIO_HIGHEST)
                priorityStr = "highest";
            
            lua_pushstring(L, priorityStr);
            rv = 1;
        }
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Get the thread's numerical id.
// @return id as a number.
// @function id
int ThreadUserdata::id(lua_State* L)
{
    int rv = 0;
    ThreadUserdata* thud = checkPrivateUserdata<ThreadUserdata>(L, 1);
    
    try
    {
        lua_Integer id = thud->mThread.id();
        lua_pushinteger(L, id);
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

/// Gets the threads running state.
// @return boolean indicating running or not.
int ThreadUserdata::isRunning(lua_State* L)
{
    int rv = 0;
    ThreadUserdata* thud = checkPrivateUserdata<ThreadUserdata>(L, 1);
    
    try
    {
        int running = thud->mThread.isRunning();
        lua_pushboolean(L, running);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Waits until the thread completes execution.
// @function join
int ThreadUserdata::join(lua_State* L)
{
    int rv = 0;
    ThreadUserdata* thud = checkPrivateUserdata<ThreadUserdata>(L, 1);
    
    try
    {
        if (thud->mStarted && !thud->mJoined)
        {
            thud->mThread.join();
            
            thud->mJoined = true;
            
            if (thud->mThreadResult != 0)
            {
                Poco::ScopedLock<Poco::FastMutex> lock(thud->mThreadMutex);
                lua_pushnil(L);
                lua_pushstring(L, thud->mErrorMsg.c_str());
                rv = 2;
            }
            else
            {
                lua_pushboolean(L, 1);
                lua_pushstring(L, "success");
                rv = 2;
            }
        }
        else
        {
            lua_pushnil(L);
            lua_pushstring(L, "trying to join an unstarted or already joined thread");
            rv = 2;
        }
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

/// Get or set the thread's stack size.
// Pass no value to get the thread's stack size.
// @int[opt] stackSize if stackSize is passed as a number, the priority will be set, otherwise the current stackSize is returned.
// @return stackSize as a number.
// @function stackSize
int ThreadUserdata::stackSize(lua_State* L)
{
    int rv = 0;
    ThreadUserdata* thud = checkPrivateUserdata<ThreadUserdata>(L, 1);
    
    int top = lua_gettop(L);
    lua_Integer stackSize = 0;
    
    if (top > 1)
        stackSize = luaL_checkinteger(L, 2);
    
    try
    {
        if (top > 1)
        {
            thud->mThread.setStackSize(stackSize);
            lua_pushboolean(L, 1);
        }
        else
        {
            stackSize = thud->mThread.getStackSize();
            lua_pushinteger(L, stackSize);
        }
        
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

/// Start a a new thread with a Lua function. 
// @param threadStart a function to be copied to the new state.  Upvalues are ignored.
// @param ... A variable list of basic Lua types or poco userdata that are noted to be copyable/sharable between threads.
// @return true or nil. (error)
// @return error message.
// @function start
int ThreadUserdata::start(lua_State* L)
{
    int rv = 0;
    int top = lua_gettop(L);
    ThreadUserdata* thud = checkPrivateUserdata<ThreadUserdata>(L, 1);
        
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    if (thud->mStarted)
    {
        lua_pushnil(L);
        lua_pushstring(L, "thread already started.");
        return 2;
    }
    
    Poco::ScopedLock<Poco::FastMutex> lock(thud->mThreadMutex);
    
    // any code that returns due to a failure will clean up the allocated state 
    // and it will not be assigned to the mState member variable.
    LuaStateHolder holder(luaL_newstate());
    luaL_openlibs(holder.state);
    if (!loadMetatables(holder.state))
    {
        lua_pushnil(L);
        lua_pushstring(L, "could not load poco library into thread's state.");
        return 2;
    }
    
    for (int i = 2; i <= top; ++i)
    {
        lua_pushvalue(L, i);
        if (!transferValue(holder.state, L))
        {
            lua_pushnil(L);
            lua_pushfstring(L, "non-copyable value at parameter %d\n", i);
            return 2;
        }
        lua_pop(L, 1);
    }
    
    try
    {
        thud->mParamCount = top - 2;
        rv = 1;
        thud->mThread.start(*thud);
        lua_pushboolean(L, 1);
        thud->mStarted = true;
    }
    catch (const Poco::Exception& e)
    {
        rv = pushPocoException(L, e);
    }
    catch (...)
    {
        rv = pushUnknownException(L);
    }
    
    // extract the state from the holder, which prevents it from being closed.
    thud->mThreadState = holder.extract();
    return rv;
}

// ThreadUserdata is a Poco::Runnable, so this is executed as part of Poco::Thread::start().
void ThreadUserdata::run()
{
    Poco::ScopedLock<Poco::FastMutex> lock(mThreadMutex);
    int top = lua_gettop(mThreadState);
    mThreadResult = lua_pcall(mThreadState, mParamCount, 0, 0);
    if (mThreadResult != 0) mErrorMsg = lua_tostring(mThreadState, -1);
}

} // LuaPoco
