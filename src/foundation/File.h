#ifndef LUA_POCO_FILE_H
#define LUA_POCO_FILE_H

#include "LuaPoco.h"
#include "Userdata.h"

namespace LuaPoco
{

/**
 * Registers the File class into the Foundation table. 
 * Assumes foundation table is at the top of the stack.
 */
bool registerFile(lua_State* L);

class FileUserdata : public Userdata
{
public:
	FileUserdata(const char *path);
	virtual ~FileUserdata();
private:
	// constructor
	static int File(lua_State* L);
	// member functions
	static int exists(lua_State* L);
	static int getFileSize(lua_State* L);
	// metamethods
	static int metamethod__gc(lua_State* L);
	static int metamethod__index(lua_State* L);
	static int metamethod__newindex(lua_State* L);
	
	Poco::File* mFile;
	
};

} // LuaPoco

#endif
