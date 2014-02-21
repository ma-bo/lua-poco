/// Generic interface from reading from ostream userdata.
// @module ostream

#include "OStream.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

bool OStream::registerOStream(lua_State* L)
{
	luaL_newmetatable(L, "Poco.OStream.metatable");
	lua_pushvalue(L, -1);
	
	// methods
	lua_pushcfunction(L, write);
	lua_setfield(L, -2, "write");
	lua_pushcfunction(L, flush);
	lua_setfield(L, -2, "flush");
	lua_pushcfunction(L, seek);
	lua_setfield(L, -2, "seek");
	lua_pop(L, 1);
	
	return true;
}

// userdata methods
int OStream::write(lua_State* L)
{
	int rv = 0;
	luaL_checktype(L, 1, LUA_TUSERDATA);
	OStream* osud = reinterpret_cast<OStream*>(lua_touserdata(L, 1));
	
	try
	{
		std::ostream& is = osud->getHandle();
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

int OStream::flush(lua_State* L)
{
	int rv = 0;
	try
	{
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

int OStream::seek(lua_State* L)
{
	int rv = 0;
	try
	{
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

} // LuaPoco
