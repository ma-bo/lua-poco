#ifndef LUA_POCO_FILE_H
#define LUA_POCO_FILE_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/File.h"

namespace LuaPoco
{

class FileUserdata : public Userdata
{
public:
	FileUserdata(const char *path);
	virtual ~FileUserdata();
	// register metatable for this class
	static bool registerFile(lua_State* L);
private:
	// constructor function 
	// Foundation.File(string) -> userdata
	static int File(lua_State* L);
	
	// userdata methods
	// File:exists() -> boolean
	static int exists(lua_State* L);
	// File:getSize -> boolean
	static int getSize(lua_State* L);
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	static int metamethod__index(lua_State* L);
	static int metamethod__newindex(lua_State* L);
	
	Poco::File* mFile;
};

} // LuaPoco

#endif
