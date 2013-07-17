#include "File.h"

#include <iostream>

namespace LuaPoco
{

bool FileUserdata::registerFile(lua_State* L)
{
	bool result = false;
	if (lua_istable(L, -1))
	{
		// register the constructor in the table
		lua_pushcfunction(L, File);
		lua_setfield(L, -2, "File");
		
		// create metatable
		int fileMtIndex = luaL_newmetatable(L, "LuaPoco.File.metatable");
		if (fileMtIndex > 0)
		{
			lua_pushcfunction(L, exists);
			lua_setfield(L, -2, "exists");
			lua_pushcfunction(L, getSize);
			lua_setfield(L, -2, "getSize");
			// This idea clearly doesn't work, will have to actually do dispatching in __index via a function
			// and then do a secondary lookup for dispatching properties
			// lua_pushcfunction(L, metamethod__index);
			// lua_setfield(L, -2, "__index");
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			lua_pushcfunction(L, metamethod__newindex);
			//lua_setfield(L, -2, "__newindex");
			//lua_pushcfunction(L, metamethod__gc);
			lua_setfield(L, -2, "__gc");
			
			lua_setmetatable(L, -2);
			
			result = true;
		}
	}
	
	return result;
}


FileUserdata::FileUserdata(const char* path) : mFile(NULL)
{
	setType(Userdata_File);
	// add exception handling here
	mFile = new Poco::File(path);
}

FileUserdata::~FileUserdata()
{
	delete mFile;
	mFile = NULL;
}

// userdata constructor
int FileUserdata::File(lua_State* L)
{
	const char* path = luaL_checkstring(L, 1);
	FileUserdata *fud = NULL;
	void* ud = lua_newuserdata(L, sizeof *fud);
	luaL_getmetatable(L, "LuaPoco.File.metatable");
	lua_setmetatable(L, -2);
	
	fud = new (ud) FileUserdata(path);
	return 1;
}

// member functions
int FileUserdata::exists(lua_State* L)
{
	void* ud = luaL_checkudata(L, 1, "LuaPoco.File.metatable");
	
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(ud);
	int exists = 0;
	
	// TODO:XXX add some exception handlers
	if (fud->mFile->exists())
		exists = 1;
	
	lua_pushboolean(L, exists);
	return 1;
}

int FileUserdata::getSize(lua_State* L)
{
	void* ud = luaL_checkudata(L, 1, "LuaPoco.File.metatable");
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(ud);
	
	// TODO:XXX add some exception handlers
	lua_Number num = fud->mFile->getSize();
	lua_pushnumber(L, num);
	return 1;
}

// metamethods
int FileUserdata::metamethod__gc(lua_State* L)
{
	void* ud = lua_touserdata(L, -1);
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(ud);
	fud->~FileUserdata();
	
	return 0;
}

int FileUserdata::metamethod__index(lua_State* L)
{
	return 0;
}

int FileUserdata::metamethod__newindex(lua_State* L)
{
	return 0;
}

} // LuaPoco
