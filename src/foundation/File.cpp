#include "File.h"
#include <Poco/Exception.h>
#include <iostream>
#include <string>

namespace LuaPoco
{

FileUserdata::FileUserdata(const char* path) : mFile(NULL)
{
	mFile = new Poco::File(path);
}

FileUserdata::FileUserdata(const Poco::File& file)
{
	mFile = new Poco::File(file);
}

FileUserdata::~FileUserdata()
{
	delete mFile;
	mFile = NULL;
}

UserdataType FileUserdata::getType()
{
	return Userdata_File;
}

bool FileUserdata::copyToState(lua_State* L)
{
	void* ud = lua_newuserdata(L, sizeof *this);
	luaL_getmetatable(L, "Poco.File.metatable");
	lua_setmetatable(L, -2);
	
	FileUserdata* fud = new(ud) FileUserdata(*mFile);
	
	return true;
}

bool FileUserdata::registerFile(lua_State* L)
{
	if (!lua_istable(L, -1))
		return false;
	
	// constructor: poco.File()
	lua_pushcfunction(L, File);
	lua_setfield(L, -2, "File");
	
	// create metatable for Poco::File
	luaL_newmetatable(L, "Poco.File.metatable");
	// indexing and gc
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
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
	lua_pushcfunction(L, canExecute);
	lua_setfield(L, -2, "canExecute");
	lua_pushcfunction(L, canRead);
	lua_setfield(L, -2, "canRead");
	lua_pushcfunction(L, canWrite);
	lua_setfield(L, -2, "canWrite");
	lua_pushcfunction(L, created);
	lua_setfield(L, -2, "created");
	lua_pushcfunction(L, exists);
	lua_setfield(L, -2, "exists");
	lua_pushcfunction(L, getLastModified);
	lua_setfield(L, -2, "lastModified");
	lua_pushcfunction(L, getSize);
	lua_setfield(L, -2, "size");
	lua_pushcfunction(L, isDevice);
	lua_setfield(L, -2, "isDevice");
	lua_pushcfunction(L, isDirectory);
	lua_setfield(L, -2, "isDirectory");
	lua_pushcfunction(L, isFile);
	lua_setfield(L, -2, "isFile");
	lua_pushcfunction(L, isHidden);
	lua_setfield(L, -2, "isHidden");
	lua_pushcfunction(L, isLink);
	lua_setfield(L, -2, "isLink");
	lua_pushcfunction(L, path);
	lua_setfield(L, -2, "path");
	lua_pushcfunction(L, setExecutable);
	lua_setfield(L, -2, "setExecutable");
	lua_pushcfunction(L, setLastModified);
	lua_setfield(L, -2, "setLastModified");
	lua_pushcfunction(L, setReadOnly);
	lua_setfield(L, -2, "setReadOnly");
	lua_pushcfunction(L, setSize);
	lua_setfield(L, -2, "setSize");
	lua_pushcfunction(L, setWritable);
	lua_setfield(L, -2, "setWritable");
	lua_pop(L, 1);
	
	return true;
}

// lua_Cfunctions registered in the lua_State
// poco.File() constructor
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
		luaL_getmetatable(L, "Poco.File.metatable");
		lua_setmetatable(L, -2);
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		return 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		return 2;
	}

	return 1;
}

// metamethods
int FileUserdata::metamethod__gc(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	fud->~FileUserdata();
	
	return 0;
}

int FileUserdata::metamethod__tostring(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	lua_pushfstring(L, "Poco.File (%p)", reinterpret_cast<void*>(fud));
	return 1;
}

// methods
int FileUserdata::copyTo(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
		
	size_t pathLen = 0;
	const char* path = luaL_checklstring(L, 2, &pathLen);
	
	if (pathLen == 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "invalid path");
		return 2;
	}
	
	try
	{
		fud->mFile->copyTo(path);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::createDirectories(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
		
	try
	{
		fud->mFile->createDirectories();
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::createDirectory(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int rv = 0;
	int created = 0;
	
	try
	{
		created = fud->mFile->createDirectory();
		lua_pushboolean(L, created);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::createFile(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int created = 0;
	
	try
	{
		created = fud->mFile->createFile();
		lua_pushboolean(L, created);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::listNames(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	std::vector<std::string> fileNames;
	
	try
	{
		fud->mFile->list(fileNames);
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		return 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		return 2;
	}
	
	lua_createtable(L, fileNames.size(), 0);
	int tableIndex = 1;
	std::vector<std::string>::iterator i = fileNames.begin();
	
	while (i != fileNames.end())
	{
		lua_pushlstring(L, i->c_str(), i->size());
		lua_rawseti(L, -2, tableIndex);
		++i;
		++tableIndex;
	}
	
	return 1;
}

int FileUserdata::listFiles(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	std::vector<Poco::File> files;
	
	try
	{
		fud->mFile->list(files);
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		return 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		return 2;
	}
	
	lua_createtable(L, files.size(), 0);
	int tableIndex = 1;
	std::vector<Poco::File>::iterator i = files.begin();
	
	while (i != files.end())
	{
		void* ud = lua_newuserdata(L, sizeof *fud);
		luaL_getmetatable(L, "Poco.File.metatable");
		lua_setmetatable(L, -2);
		FileUserdata* fud = new(ud) FileUserdata(*i);
		lua_rawseti(L, -2, tableIndex);
		++i;
		++tableIndex;
	}
	
	return 1;
}

int FileUserdata::moveTo(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
		
	size_t pathLen = 0;
	const char* path = luaL_checklstring(L, 2, &pathLen);
	
	if (pathLen == 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "invalid path");
		return 2;
	}
	
	try
	{
		fud->mFile->moveTo(path);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::remove(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	// default false as per defaul parameter to Poco::File::remove().
	int recursive = 0;
	if (lua_gettop(L) > 1)
		recursive = lua_toboolean(L, 2);
	
	try
	{
		fud->mFile->remove(recursive);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::renameTo(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
		
	size_t pathLen = 0;
	const char* path = luaL_checklstring(L, 2, &pathLen);
	
	if (pathLen == 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "invalid path");
		return 2;
	}
	
	try
	{
		fud->mFile->renameTo(path);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::canExecute(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int executable = 0;
	try
	{
		executable = fud->mFile->canExecute();
		lua_pushboolean(L, executable);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::canRead(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int readable = 0;
	try
	{
		readable = fud->mFile->canRead();
		lua_pushboolean(L, readable);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::canWrite(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int writable = 0;
	try
	{
		writable = fud->mFile->canWrite();
		lua_pushboolean(L, writable);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::created(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	return 0;
}

int FileUserdata::exists(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	int exists = 0;
	try
	{
		exists = fud->mFile->exists();
		lua_pushboolean(L, exists);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::getLastModified(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	return 0;
}

int FileUserdata::getSize(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	lua_Number num;
	
	try
	{
		num = fud->mFile->getSize();
		lua_pushnumber(L, num);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::isDevice(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int device = 0;
	try
	{
		device = fud->mFile->isDevice();
		lua_pushboolean(L, device);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::isDirectory(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int directory = 0;
	try
	{
		directory = fud->mFile->isDirectory();
		lua_pushboolean(L, directory);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::isFile(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int file = 0;
	try
	{
		file = fud->mFile->isFile();
		lua_pushboolean(L, file);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::isHidden(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int hidden = 0;
	try
	{
		hidden = fud->mFile->isHidden();
		lua_pushboolean(L, hidden);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::isLink(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int link = 0;
	try
	{
		link = fud->mFile->isLink();
		lua_pushboolean(L, link);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::path(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	std::string path = fud->mFile->path();
	lua_pushlstring(L, path.c_str(), path.size());
	
	return 1;
}

int FileUserdata::setExecutable(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	int executable = 1;
	if (lua_gettop(L) > 1)
		executable = lua_toboolean(L, 2);
	try
	{
		fud->mFile->setExecutable(executable);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::setLastModified(lua_State* L)
{
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	return 0;
}

int FileUserdata::setReadOnly(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
	
	
	int readOnly = 1;
	if (lua_gettop(L) > 1)
		readOnly = lua_toboolean(L, 2);
	
	try
	{
		fud->mFile->setReadOnly(readOnly);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::setSize(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
		
	lua_Number size = luaL_checknumber(L, 2);
	
	try
	{
		fud->mFile->setSize(size);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

int FileUserdata::setWritable(lua_State* L)
{
	int rv = 0;
	FileUserdata* fud = reinterpret_cast<FileUserdata*>(
		luaL_checkudata(L, 1, "Poco.File.metatable"));
		
	int writable = 1;
	if (lua_gettop(L) > 1)
		writable = lua_toboolean(L, 2);
	
	try
	{
		fud->mFile->setWriteable(writable);
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		lua_pushnil(L);
		lua_pushstring(L, e.what());
		rv = 2;
	}
	catch (...)
	{
		lua_pushnil(L);
		lua_pushstring(L, "unknown error");
		rv = 2;
	}
	
	return rv;
}

} // LuaPoco
