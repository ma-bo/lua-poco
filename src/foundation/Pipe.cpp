#include "Pipe.h"
#include "Poco/Exception.h"
#include <cstring>

namespace LuaPoco
{

PipeUserdata::PipeUserdata() :
	mPipe(), mReadStream(mPipe), mWriteStream(mPipe)
{
}

PipeUserdata::PipeUserdata(const Poco::Pipe& p) :
	mPipe(p), mReadStream(mPipe), mWriteStream(mPipe)
{
}

PipeUserdata::~PipeUserdata()
{
}

UserdataType PipeUserdata::getType()
{
	return Userdata_Pipe;
}

bool PipeUserdata::isCopyable()
{
	return true;
}

bool PipeUserdata::copyToState(lua_State *L)
{
	void* ud = lua_newuserdata(L, sizeof(PipeUserdata));
	luaL_getmetatable(L, "Poco.Pipe.metatable");
	lua_setmetatable(L, -2);
	
	PipeUserdata* pud = new(ud) PipeUserdata();
	return true;
}

// register metatable for this class
bool PipeUserdata::registerPipe(lua_State* L)
{
	bool result = false;
	if (!lua_istable(L, -1))
		return result;
	
	// constructor
	lua_pushcfunction(L, Pipe);
	lua_setfield(L, -2, "Pipe");
	
	luaL_newmetatable(L, "Poco.Pipe.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushstring(L, "Poco.Pipe.metatable");
	lua_setfield(L, -2, "poco.userdata");
	
	// methods
	lua_pushcfunction(L, read);
	lua_setfield(L, -2, "read");
	lua_pushcfunction(L, write);
	lua_setfield(L, -2, "write");
	lua_pushcfunction(L, close);
	lua_setfield(L, -2, "close");
	lua_pop(L, 1);
	result = true;
	
	return result;
}

int PipeUserdata::Pipe(lua_State* L)
{
	void* ud = lua_newuserdata(L, sizeof(PipeUserdata));
	luaL_getmetatable(L, "Poco.Pipe.metatable");
	lua_setmetatable(L, -2);
	
	PipeUserdata* pud = new(ud) PipeUserdata();
	return 1;
}

// metamethod infrastructure
int PipeUserdata::metamethod__gc(lua_State* L)
{
	PipeUserdata* pud = reinterpret_cast<PipeUserdata*>(
		luaL_checkudata(L, 1, "Poco.Pipe.metatable"));
	pud->~PipeUserdata();
	
	return 0;
}

int PipeUserdata::metamethod__tostring(lua_State* L)
{
	PipeUserdata* pud = reinterpret_cast<PipeUserdata*>(
		luaL_checkudata(L, 1, "Poco.Pipe.metatable"));
	
	lua_pushfstring(L, "Poco.Pipe (%p)", reinterpret_cast<void*>(pud));
	return 1;
}

// userdata methods
int PipeUserdata::read(lua_State* L)
{
	int rv = 0;
	PipeUserdata* pud = reinterpret_cast<PipeUserdata*>(
		luaL_checkudata(L, 1, "Poco.Pipe.metatable"));
	
	size_t readAmount = 0;
	const char* readType = NULL;
	
	if (lua_gettop(L) > 1)
	{
		if (lua_isnumber(L, 2))
			readAmount = lua_tonumber(L, 2);
		else
			readType = lua_tostring(L, 2);
	}
	
	if (readAmount > 0)
	{
		luaL_Buffer lb;
		luaL_buffinit(L, &lb);
		
		size_t total = 0;
		char buffer[500];
		do
		{
			size_t toRead = readAmount > sizeof buffer ? sizeof buffer : readAmount;
			pud->mReadStream.read(buffer, toRead);
			if (pud->mReadStream.good())
			{
				luaL_addlstring(&lb, buffer, toRead);
				readAmount -= toRead;
			}
			else
				break;
		} while (readAmount > 0);
		luaL_pushresult(&lb);
		
		if (pud->mReadStream.good())
		{
			rv = 1;
		}
		else if (pud->mReadStream.eof())
		{
			lua_pushnil(L);
			lua_pushstring(L, "eof");
			rv = 2;
		}
		else
		{
			lua_pushnil(L);
			lua_pushstring(L, "failed to write to pipe");
			rv = 2;
		}
	}
	else if (readType && std::strcmp(readType, "*l") != 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "invalid read format.");
		rv = 2;
	}
	else
	{
		std::string line;
		if (std::getline(pud->mReadStream, line).good())
		{
			lua_pushlstring(L, line.data(), line.size());
			rv = 1;
		}
	}
	
	return rv;
}

int PipeUserdata::write(lua_State* L)
{
	int rv = 0;
	PipeUserdata* pud = reinterpret_cast<PipeUserdata*>(
		luaL_checkudata(L, 1, "Poco.Pipe.metatable"));

	size_t strSize = 0;
	const char* str = luaL_checklstring(L, 2, &strSize);
	size_t startIdx = 0;
	size_t endIdx = strSize;
	
	int top = lua_gettop(L);
	if (top > 2)
	{
		lua_Integer startPos = luaL_checkinteger(L, 3) - 1;
		startIdx = startPos < 0 ? 0 : startPos;
		if (top > 3)
		{
			endIdx = luaL_checkinteger(L, 4);
			endIdx = endIdx > strSize ? strSize : endIdx;
		}
	}
	
	try
	{
		pud->mWriteStream.write(&str[startIdx], endIdx - startIdx);
		if (pud->mWriteStream.good())
		{
			lua_pushboolean(L, 1);
			rv = 1;
		}
		else if (pud->mWriteStream.eof())
		{
			lua_pushnil(L);
			lua_pushstring(L, "eof");
			rv = 2;
		}
		else
		{
			lua_pushnil(L);
			lua_pushstring(L, "failed to write to pipe");
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

int PipeUserdata::close(lua_State* L)
{
	int rv = 0;
	PipeUserdata* pud = reinterpret_cast<PipeUserdata*>(
		luaL_checkudata(L, 1, "Poco.Pipe.metatable"));
	
	const char* closeEnd = "both";
	int top = lua_gettop(L);
	if (top > 1)
		closeEnd = luaL_checkstring(L, 2);
	
	if (std::strcmp(closeEnd, "read") == 0)
		pud->mReadStream.close();
	else if (std::strcmp(closeEnd, "write") == 0)
		pud->mWriteStream.close();
	else
	{
		pud->mReadStream.close();
		pud->mWriteStream.close();
	}
	
	return 1;
}

} // LuaPoco
