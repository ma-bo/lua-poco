#ifndef LUA_POCO_REGULAREXPRESSION_H
#define LUA_POCO_REGULAREXPRESSION_H

#include <string>
#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/RegularExpression.h"

namespace LuaPoco
{

class RegularExpressionUserdata : public Userdata
{
public:
	RegularExpressionUserdata(const std::string& pattern, int options, bool study);
	virtual ~RegularExpressionUserdata();
	virtual UserdataType getType();
	
	// register metatable for this class
	static bool registerRegularExpression(lua_State* L);
private:
	RegularExpressionUserdata();
	RegularExpressionUserdata(const RegularExpressionUserdata& disable);
	RegularExpressionUserdata operator=(const RegularExpressionUserdata& disable);
	
	// constructor function 
	static int RegularExpression(lua_State* L);
	
	// metamethod infrastructure
	static int metamethod__gc(lua_State* L);
	static int metamethod__tostring(lua_State* L);
	
	// userdata methods
	static int extract(lua_State* L);
	static int match(lua_State* L);
	static int extractPositions(lua_State* L);
	static int extractCaptures(lua_State* L);
	static int substitute(lua_State* L);
	
	Poco::RegularExpression mRegularExpression;
};

} // LuaPoco

#endif
