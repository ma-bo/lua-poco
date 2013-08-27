#include "Thread.h"
#include "StateTransfer.h"
#include "Poco/Exception.h"
#include <cstring>
#include <iostream>

namespace LuaPoco
{

ThreadUserdata::ThreadUserdata() :
	mThread(), mJoined(false), mStarted(false), mThreadState(NULL), mParamCount(0)
{
}

ThreadUserdata::~ThreadUserdata()
{
	if (mStarted && !mJoined)
		mThread.join();
	if (mThreadState)
		lua_close(mThreadState);
}

UserdataType ThreadUserdata::getType()
{
	return Userdata_Thread;
}

// register metatable for this class
bool ThreadUserdata::registerThread(lua_State* L)
{
	bool result = false;
	if (!lua_istable(L, -1))
		return result;
	
	// constructor
	lua_pushcfunction(L, Thread);
	lua_setfield(L, -2, "Thread");
	
	luaL_newmetatable(L, "Poco.Thread.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushstring(L, "Poco.Thread.metatable");
	lua_setfield(L, -2, "poco.userdata");

	// methods
	lua_pushcfunction(L, name);
	lua_setfield(L, -2, "name");
	lua_pushcfunction(L, id);
	lua_setfield(L, -2, "id");
	lua_pushcfunction(L, isRunning);
	lua_setfield(L, -2, "isRunning");
	lua_pushcfunction(L, join);
	lua_setfield(L, -2, "join");
	lua_pushcfunction(L, stackSize);
	lua_setfield(L, -2, "stackSize");
	lua_pushcfunction(L, start);
	lua_setfield(L, -2, "start");
	lua_pushcfunction(L, priority);
	lua_setfield(L, -2, "priority");
	
	lua_pop(L, 1);
	result = true;
	
	return result;
}

int ThreadUserdata::Thread(lua_State* L)
{
	int rv = 0;
	int top = lua_gettop(L);
	
	const char* priority = NULL;
	const char* name = NULL;
	lua_Integer stackSize = 0;
	
	if (top > 0)
		priority = luaL_checkstring(L, 1);
	if (top > 1)
		name = luaL_checkstring(L, 2);
	if (top > 2)
		stackSize = luaL_checkinteger(L, 3);

	try
	{
		void* ud = lua_newuserdata(L, sizeof(ThreadUserdata));
		luaL_getmetatable(L, "Poco.Thread.metatable");
		lua_setmetatable(L, -2);
		ThreadUserdata* thud = new(ud) ThreadUserdata();
		rv = 1;
		
		if (top > 0 && priority)
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
		}
		
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

// metamethod infrastructure
int ThreadUserdata::metamethod__gc(lua_State* L)
{
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
	thud->~ThreadUserdata();
	
	return 0;
}

int ThreadUserdata::metamethod__tostring(lua_State* L)
{
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
	
	lua_pushfstring(L, "Poco.Thread (%p)", reinterpret_cast<void*>(thud));
	return 1;
}

// userdata methods
int ThreadUserdata::name(lua_State* L)
{
	int rv = 0;
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
	
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
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

int ThreadUserdata::priority(lua_State* L)
{
	int rv = 0;
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
	
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
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

int ThreadUserdata::id(lua_State* L)
{
	int rv = 0;
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
	
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

int ThreadUserdata::isRunning(lua_State* L)
{
	int rv = 0;
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
	
	try
	{
		int running = thud->mThread.isRunning();
		lua_pushboolean(L, running);
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

int ThreadUserdata::join(lua_State* L)
{
	int rv = 0;
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
		
	int top = lua_gettop(L);
	int ms = 0;
	
	if (top > 1)
		ms = luaL_checkinteger(L, 2);
	
	try
	{
		if (top > 1)
			thud->mThread.join(ms);
		else
			thud->mThread.join();
		
		thud->mJoined = true;
		lua_pushboolean(L, 1);
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

int ThreadUserdata::stackSize(lua_State* L)
{
	int rv = 0;
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
	
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

int ThreadUserdata::start(lua_State* L)
{
	int rv = 0;
	int top = lua_gettop(L);
	ThreadUserdata* thud = reinterpret_cast<ThreadUserdata*>(
		luaL_checkudata(L, 1, "Poco.Thread.metatable"));
		
	luaL_checktype(L, 2, LUA_TFUNCTION);
	
	thud->mThreadState = luaL_newstate();
	luaL_openlibs(thud->mThreadState);
	if (luaL_dostring(thud->mThreadState, "return require('poco')") != 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "could not load poco library into thread's state.");
		return 2;
	}
	
	for (int i = 2; i <= top; ++i)
	{
		lua_pushvalue(L, i);
		if (!transferValue(thud->mThreadState, L))
		{
			lua_pushnil(L);
			lua_pushfstring(L, "non-copyable value at parameter %d\n", i);
			return 2;
		}
		const char* msg = lua_tostring(L, -1);
		msg = msg ? msg : "NULL";
		std::cout << "pushed index: " << i << " of type: " << msg << std::endl;
		lua_pop(L, 1);
	}
	
	try
	{
		thud->mParamCount = top - 2;
		rv = 1;
		thud->mThread.start(*thud);
		lua_pushboolean(L, 1);
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

void ThreadUserdata::run()
{
	const char* msg;
	std::cout << "calling function with " << mParamCount << " args." << std::endl;
	int top = lua_gettop(mThreadState);
	
	for (int i = 1; i < top; ++i)
	{
		std::cout << "type of value at " << i << ": " << lua_typename(mThreadState, lua_type(mThreadState, i)) << std::endl;
	}
	
	int rv = lua_pcall(mThreadState, mParamCount, 0, 0);
	if (rv == 0)
		std::cout << "ThreadUserdata::run success!" << std::endl;
	else
	{
		std::cout << "ThreadUserdata::run pcall returned error!" << std::endl;
		std::cout << "error: " << lua_tostring(mThreadState, -1) << std::endl;
	}
}

} // LuaPoco
