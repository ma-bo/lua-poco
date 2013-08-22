#ifndef LUA_POCO_PIPE_H
#define LUA_POCO_PIPE_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/Pipe.h"
#include "Poco/PipeStream.h"

namespace LuaPoco
{

class PipeUserdata : public Userdata
{
public:
	PipeUserdata();
	PipeUserdata(const Poco::Pipe& p);
	virtual ~PipeUserdata();
	virtual UserdataType getType();
	virtual bool isCopyable();
	virtual bool copyToState(lua_State *L);
	// register metatable for this class
	static bool registerPipe(lua_State* L);
	Poco::Pipe mPipe;
private:
	// constructor function 
	static int Pipe(lua_State* L);
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	static int metamethod__tostring(lua_State* L);
	
	// userdata methods
	static int read(lua_State* L);
	static int write(lua_State* L);
	static int close(lua_State* L);
	
	Poco::PipeInputStream mReadStream;
	Poco::PipeOutputStream mWriteStream;
};

} // LuaPoco

#endif
