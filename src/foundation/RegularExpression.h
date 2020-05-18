#ifndef LUA_POCO_REGULAREXPRESSION_H
#define LUA_POCO_REGULAREXPRESSION_H

#include <string>
#include "Userdata.h"
#include <Poco/RegularExpression.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_regex(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_REGULAREXPRESSION_METATABLE_NAME;

class RegularExpressionUserdata : public Userdata
{
public:
    RegularExpressionUserdata(const std::string& pattern, int options, bool study);
    virtual ~RegularExpressionUserdata();
    
    // register metatable for this class
    static bool registerRegularExpression(lua_State* L);
    // constructor function 
    static int RegularExpression(lua_State* L);
private:
    RegularExpressionUserdata();
    RegularExpressionUserdata(const RegularExpressionUserdata& disable);
    RegularExpressionUserdata operator=(const RegularExpressionUserdata& disable);
    
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    
    // iterator fucntion for gmatch
    static int gmatch_iter(lua_State* L);
    // pattern compatible userdata methods
    static int find(lua_State* L);
    static int match(lua_State* L);
    static int gmatch(lua_State* L);
    static int gsub(lua_State* L);
    
    Poco::RegularExpression mRegularExpression;
};

} // LuaPoco

#endif

