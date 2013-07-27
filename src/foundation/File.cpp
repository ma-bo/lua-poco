#include "File.h"
#include <Poco/Exception.h>
#include <iostream>

namespace LuaPoco
{

FileUserdata::FileUserdata(const char* path) : mFile(NULL)
{
	setType(Userdata_File);
	setBaseType(BaseType_None);
	mFile = new Poco::File(path);
}

FileUserdata::FileUserdata(const Poco::File& file)
{
	setType(Userdata_File);
	setBaseType(BaseType_None);
	mFile = new Poco::File(file);
}

FileUserdata::~FileUserdata()
{
	delete mFile;
	mFile = NULL;
}

bool FileUserdata::registerFile(lua_State* L)
{
	bool result = false;
	if (lua_istable(L, -1))
	{
		// register the constructor in the Foundation table
		lua_pushcfunction(L, File);
		lua_setfield(L, -2, "File");
		
		// create metatable for Poco::File
		luaL_newmetatable(L, "LuaPoco.File.metatable");
		// indexing and gc
		lua_pushcfunction(L, metamethod__index);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, metamethod__newindex);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, metamethod__gc);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, metamethod__tostring);
		lua_setfield(L, -2, "__tostring");
		// methods
		lua_pushcfunction(L, copyTo);
		lua_setfield(L, -2, "copyTo");
		lua_pushcfunction(L, createDirectories);
		lua_setfield(L, -2, "createDirectories");
		lua_pushcfunction(L, createDirectory);
		lua_setfield(L, -2, "createDirectory");
		lua_pushcfunction(L, createFile);
		lua_setfield(L, -2, "createFile");
		lua_pushcfunction(L, listNames);
		lua_setfield(L, -2, "listNames");
		lua_pushcfunction(L, listFiles);
		lua_setfield(L, -2, "listFiles");
		lua_pushcfunction(L, moveTo);
		lua_setfield(L, -2, "moveTo");
		lua_pushcfunction(L, remove);
		lua_setfield(L, -2, "remove");
		lua_pushcfunction(L, renameTo);
		lua_setfield(L, -2, "renameTo");
		lua_pop(L, 1);
		
		// create table for readable properties
		luaL_newmetatable(L, "LuaPoco.File.properties.read");		
		lua_pushcfunction(L, canExecute);
		lua_setfield(L, -2, "executable");
		lua_pushcfunction(L, canRead);
		lua_setfield(L, -2, "readable");
		lua_pushcfunction(L, canWrite);
		lua_setfield(L, -2, "writable");
		lua_pushcfunction(L, created);
		lua_setfield(L, -2, "created");
		lua_pushcfunction(L, exists);
		lua_setfield(L, -2, "exists");
		lua_pushcfunction(L, getLastModified);
		lua_setfield(L, -2, "lastModified");
		lua_pushcfunction(L, getSize);
		lua_setfield(L, -2, "size");
		lua_pushcfunction(L, isDevice);
		lua_setfield(L, -2, "device");
		lua_pushcfunction(L, isDirectory);
		lua_setfield(L, -2, "directory");
		lua_pushcfunction(L, isFile);
		lua_setfield(L, -2, "file");
		lua_pushcfunction(L, isHidden);
		lua_setfield(L, -2, "hidden");
		lua_pushcfunction(L, isLink);
		lua_setfield(L, -2, "link");
		lua_pushcfunction(L, path);
		lua_setfield(L, -2, "path");
		lua_pop(L, 1);
		
		// create table for readable properties
		luaL_newmetatable(L, "LuaPoco.File.properties.write");
		lua_pushcfunction(L, setExecutable);
		lua_setfield(L, -2, "executable");
		lua_pushcfunction(L, setLastModified);
		lua_setfield(L, -2, "lastModified");
		lua_pushcfunction(L, setReadOnly);
		lua_setfield(L, -2, "readOnly");
		lua_pushcfunction(L, setSize);
		lua_setfield(L, -2, "size");
		lua_pushcfunction(L, setWritable);
		lua_setfield(L, -2, "writable");
		lua_pop(L, 1);
				
		result = true;
	}
	
	return result;
}

// functions registered in the lua_State
// foundation.File() constructor
int FileUserdata::File(lua_State* L)
{
	size_t pathLen = 0;
	const char* path = luaL_checklstring(L, 1, &pathLen);
	
	if (pathLen == 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "invalid path");
		return 2;
	}
	
	FileUserdata *fud;
	void* ud = lua_newuserdata(L, sizeof *fud);
	
	try
	{
		fud = new (ud) FileUserdata(path);
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		return 2;
	}
	
	luaL_getmetatable(L, "LuaPoco.File.metatable");
	lua_setmetatable(L, -2);
	return 1;
}

// metamethods
int FileUserdata::metamethod__gc(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "LuaPoco.File.metatable"));
	fud->~FileUserdata();
	
	return 0;
}

int FileUserdata::metamethod__index(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "LuaPoco.File.metatable"));
	
	if (!lua_getmetatable(L, 1))
		return 0;
	
	// return requested method
	lua_pushvalue(L, 2);
	lua_rawget(L, -2);
	if (!lua_isnil(L, -1))
		return 1;
	
	// else return value for property or nil if not found
	luaL_getmetatable(L, "LuaPoco.File.properties.read");
	lua_pushvalue(L, 2);
	lua_rawget(L, 1);
	
	return 1;
}

int FileUserdata::metamethod__newindex(lua_State* L)
{
	return 0;
}

int FileUserdata::metamethod__tostring(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "LuaPoco.File.metatable"));
	
	lua_pushfstring(L, "LuaPoco.File (%p)", reinterpret_cast<void*>(fud));
	return 1;
}

// methods
int FileUserdata::copyTo(lua_State* L)
{
	return 0;
}

int FileUserdata::createDirectories(lua_State* L)
{
	return 0;
}

int FileUserdata::createDirectory(lua_State* L)
{
	return 0;
}

int FileUserdata::createFile(lua_State* L)
{
	return 0;
}

int FileUserdata::listNames(lua_State* L)
{
	return 0;
}

int FileUserdata::listFiles(lua_State* L)
{
	return 0;
}

int FileUserdata::moveTo(lua_State* L)
{
	return 0;
}

int FileUserdata::remove(lua_State* L)
{
	return 0;
}
int FileUserdata::renameTo(lua_State* L)
{
	return 0;
}

// read properties
int FileUserdata::canExecute(lua_State* L)
{
	return 0;
}

int FileUserdata::canRead(lua_State* L)
{
	return 0;
}

int FileUserdata::canWrite(lua_State* L)
{
	return 0;
}

int FileUserdata::created(lua_State* L)
{
	return 0;
}

int FileUserdata::exists(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "LuaPoco.File.metatable"));
	
	int exists = 0;
	try
	{
		exists = fud->mFile->exists();
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		return 2;
	}
	
	lua_pushboolean(L, exists);
	return 1;
}

int FileUserdata::getLastModified(lua_State* L)
{
	return 0;
}

int FileUserdata::getSize(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "LuaPoco.File.metatable"));
	
	lua_Number num;
	
	try
	{
		num = fud->mFile->getSize();
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		return 2;
	}
	
	lua_pushnumber(L, num);
	return 1;
}

int FileUserdata::isDevice(lua_State* L)
{
	return 0;
}

int FileUserdata::isDirectory(lua_State* L)
{
	return 0;
}

int FileUserdata::isFile(lua_State* L)
{
	return 0;
}

int FileUserdata::isHidden(lua_State* L)
{
	return 0;
}

int FileUserdata::isLink(lua_State* L)
{
	return 0;
}

int FileUserdata::path(lua_State* L)
{
	return 0;
}

// write properties
int FileUserdata::setExecutable(lua_State* L)
{
	return 0;
}

int FileUserdata::setLastModified(lua_State* L)
{
	return 0;
}

int FileUserdata::setReadOnly(lua_State* L)
{
	return 0;
}

int FileUserdata::setSize(lua_State* L)
{
	return 0;
}

int FileUserdata::setWritable(lua_State* L)
{
	return 0;
}

} // LuaPoco
