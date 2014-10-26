/// Static typed values.
// A userdata object that can hold POCO static types.  The main reason for this class is to have a mechanism to handle 64-bit integral values used by other classes in the POCO binding, like timestamp.
//
// Notes:
//
// * dynamicany userdata is copyable between poco threads.
//
// * dynamicany userdata can be used with the Lua arithmetic and equality operators.
// 
// @module dynamicany

#include "DynamicAny.h"

int luaopen_poco_dynamicany(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::DynamicAnyUserdata::DynamicAny);
}

namespace LuaPoco
{

DynamicAnyUserdata::DynamicAnyUserdata(const Poco::DynamicAny& da) : mDynamicAny(da)
{
}

DynamicAnyUserdata::~DynamicAnyUserdata()
{
}

bool DynamicAnyUserdata::copyToState(lua_State* L)
{
    void* ud = lua_newuserdata(L, sizeof *this);
    luaL_getmetatable(L, "Poco.DynamicAny.metatable");
    lua_setmetatable(L, -2);
    
    DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(mDynamicAny);
    
    return true;
}

// register metatable for this class
bool DynamicAnyUserdata::registerDynamicAny(lua_State* L)
{
    luaL_newmetatable(L, "Poco.DynamicAny.metatable");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, metamethod__gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, metamethod__tostring);
    lua_setfield(L, -2, "__tostring");
    
    lua_pushstring(L, "Poco.DynamicAny.metatable");
    lua_setfield(L, -2, "poco.userdata");
    
    // methods
    lua_pushcfunction(L, convert);
    lua_setfield(L, -2, "convert");
    lua_pushcfunction(L, isNumeric);
    lua_setfield(L, -2, "isNumeric");
    lua_pushcfunction(L, isInteger);
    lua_setfield(L, -2, "isInteger");
    lua_pushcfunction(L, isSigned);
    lua_setfield(L, -2, "isSigned");
    lua_pushcfunction(L, isString);
    lua_setfield(L, -2, "isString");
    lua_pushcfunction(L, toNumber);
    lua_setfield(L, -2, "toNumber");
    lua_pushcfunction(L, toString);
    lua_setfield(L, -2, "toString");
    lua_pushcfunction(L, toBoolean);
    lua_setfield(L, -2, "toBoolean");
    lua_pushcfunction(L, metamethod__add);
    lua_setfield(L, -2, "__add");
    lua_pushcfunction(L, metamethod__sub);
    lua_setfield(L, -2, "__sub");
    lua_pushcfunction(L, metamethod__mul);
    lua_setfield(L, -2, "__mul");
    lua_pushcfunction(L, metamethod__div);
    lua_setfield(L, -2, "__div");
    lua_pushcfunction(L, metamethod__eq);
    lua_setfield(L, -2, "__eq");
    lua_pushcfunction(L, metamethod__lt);
    lua_setfield(L, -2, "__lt");
    lua_pushcfunction(L, metamethod__le);
    lua_setfield(L, -2, "__le");
    lua_pop(L, 1);
    
    return true;
}

/// construct a new userdata from value.
// @param value of types number, string, boolean, or dynamicany userdata.
// @return userdata or nil. (error)
// @return error message.
// @function new
int DynamicAnyUserdata::DynamicAny(lua_State* L)
{
    int rv = 0;
    
    luaL_checkany(L, 1);
    int type = lua_type(L, 1);
    try
    {
        if (type == LUA_TNUMBER)
        {
            void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
            luaL_getmetatable(L, "Poco.DynamicAny.metatable");
            lua_setmetatable(L, -2);
            lua_Number val = lua_tonumber(L, 1);
            DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(val);
            rv = 1;
        }
        else if (type == LUA_TSTRING)
        {
            void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
            luaL_getmetatable(L, "Poco.DynamicAny.metatable");
            lua_setmetatable(L, -2);
            const char* val = lua_tostring(L, 1);
            DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(val);
            rv = 1;
        }
        else if (type == LUA_TBOOLEAN)
        {
            void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
            luaL_getmetatable(L, "Poco.DynamicAny.metatable");
            lua_setmetatable(L, -2);
            bool val = lua_toboolean(L, 1);
            DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(val);
            rv = 1;
        }
        else if (type == LUA_TUSERDATA)
        {
            void *udFrom = luaL_checkudata(L, 1, "Poco.DynamicAny.metatable");
            void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
            luaL_getmetatable(L, "Poco.DynamicAny.metatable");
            lua_setmetatable(L, -2);
            DynamicAnyUserdata* daudFrom = reinterpret_cast<DynamicAnyUserdata*>(udFrom);
            DynamicAnyUserdata* daud = new(ud) DynamicAnyUserdata(*daudFrom);
            rv = 1;
        }
        else
        {
            lua_pushnil(L);
            lua_pushstring(L,"DynamicAny requires a number, string, boolean, or a Poco.DynamicAny");
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

///
// @type dynamicany

// metamethod infrastructure
int DynamicAnyUserdata::metamethod__gc(lua_State* L)
{
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    daud->~DynamicAnyUserdata();
    
    return 0;
}

int DynamicAnyUserdata::metamethod__tostring(lua_State* L)
{
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    
    lua_pushfstring(L, "Poco.DynamicAny (%p)", reinterpret_cast<void*>(daud));
    return 1;
}

// methods

/// converts value to new type.
// @string toType  "UInt64", "Int64", "UInt32", "Int32", "UInt16", "Int16", "UInt8", "Int8", "double", "float", "string", "bool".
// @return new userdata or nil. (error)
// @return error message.
// @function convert
int DynamicAnyUserdata::convert(lua_State* L)
{
    int rv = 0;
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    const char* toTypeStr = luaL_checkstring(L, 2);
    std::string toType(toTypeStr);
    
    try
    {
        Poco::DynamicAny newValue;
        
        if (toType == "UInt64")
            newValue = daud->mDynamicAny.convert<Poco::UInt64>();
        else if (toType == "Int64")
            newValue = daud->mDynamicAny.convert<Poco::Int64>();
        else if (toType == "UInt32")
            newValue = daud->mDynamicAny.convert<Poco::UInt32>();
        else if (toType == "Int32")
            newValue = daud->mDynamicAny.convert<Poco::Int32>();
        else if (toType == "UInt16")
            newValue = daud->mDynamicAny.convert<Poco::UInt16>();
        else if (toType == "Int16")
            newValue = daud->mDynamicAny.convert<Poco::Int16>();
        else if (toType == "UInt8")
            newValue = daud->mDynamicAny.convert<Poco::UInt8>();
        else if (toType == "Int8")
            newValue = daud->mDynamicAny.convert<Poco::Int8>();
        else if (toType == "double")
            newValue = daud->mDynamicAny.convert<double>();
        else if (toType == "float")
            newValue = daud->mDynamicAny.convert<float>();
        else if (toType == "string")
            newValue = daud->mDynamicAny.convert<std::string>();
        else if (toType == "bool")
            newValue = daud->mDynamicAny.convert<bool>();
        else
        {
            lua_pushnil(L);
            lua_pushfstring(L, "invalid conversion target: %s", toTypeStr);
            return 2;
        }
        
        void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
        luaL_getmetatable(L, "Poco.DynamicAny.metatable");
        lua_setmetatable(L, -2);
        DynamicAnyUserdata* newValueUd = new(ud) DynamicAnyUserdata(newValue);
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

/// checks if value is a numeric type.
// @return boolean
// @function isNumeric
int DynamicAnyUserdata::isNumeric(lua_State* L)
{
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    
    int isnumeric = daud->mDynamicAny.isNumeric();
    lua_pushboolean(L, isnumeric);
    
    return 1;
}

/// checks if value is an integral type.
// @return boolean
// @function isInteger
int DynamicAnyUserdata::isInteger(lua_State* L)
{
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    
    int isinteger = daud->mDynamicAny.isInteger();
    lua_pushboolean(L, isinteger);
    
    return 1;
}

/// checks if value is a signed type.
// @return boolean. (false if unsigned)
// @function isSigned
int DynamicAnyUserdata::isSigned(lua_State* L)
{
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    
    int issigned = daud->mDynamicAny.isSigned();
    lua_pushboolean(L, issigned);
    
    return 1;
}

int DynamicAnyUserdata::isString(lua_State* L)
{
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    
    int isstring = daud->mDynamicAny.isString();
    lua_pushboolean(L, isstring);
    
    return 1;
}

/// converts value to a Lua number.
// @return number or nil. (error)
// @return error message.
// @function toNumber
int DynamicAnyUserdata::toNumber(lua_State* L)
{
    int rv = 0;
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    try
    {
        lua_Number result;
        daud->mDynamicAny.convert(result);
        lua_pushnumber(L, result);
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

/// converts value to a Lua string.
// @return string or nil. (error)
// @return error message.
// @function toString
int DynamicAnyUserdata::toString(lua_State* L)
{
    int rv = 0;
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    
    try
    {
        std::string result;
        daud->mDynamicAny.convert(result);
        lua_pushlstring(L, result.c_str(), result.size());
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

/// converts value to a Lua boolean.
// @return boolean or nil. (error)
// @return error message.
// @function toBoolean
int DynamicAnyUserdata::toBoolean(lua_State* L)
{
    int rv = 0;
    DynamicAnyUserdata* daud = reinterpret_cast<DynamicAnyUserdata*>(
        luaL_checkudata(L, 1, "Poco.DynamicAny.metatable"));
    
    try
    {
        bool result;
        daud->mDynamicAny.convert(result);
        lua_pushboolean(L, result);
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


int DynamicAnyUserdata::metamethod__add(lua_State* L)
{
    int rv = 0;
    int udIndex = 1;
    int valIndex = 2;
    
    DynamicAnyUserdata* daud;
    
    if (!lua_isuserdata(L, 1))
    {
        udIndex = 2;
        valIndex = 1;
    }
    
    daud = reinterpret_cast<DynamicAnyUserdata*>(luaL_checkudata(L, udIndex, "Poco.DynamicAny.metatable"));
    
    try
    {
        void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
        luaL_getmetatable(L, "Poco.DynamicAny.metatable");
        lua_setmetatable(L, -2);
        DynamicAnyUserdata* newValueUd = new(ud) DynamicAnyUserdata(Poco::DynamicAny());
        
        if (lua_isuserdata(L, valIndex))
        {
            lua_getmetatable(L, udIndex);
            lua_getmetatable(L, valIndex);
            int equal = lua_rawequal(L, -1, -2);
            lua_pop(L, 2);
            
            if (equal)
            {
                DynamicAnyUserdata* daudOther = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, valIndex));
                newValueUd->mDynamicAny = daud->mDynamicAny + daudOther->mDynamicAny;
                rv = 1;
            }
            else
                luaL_error(L, "expected two Poco.DynamicAny userdata");
        }
        else if (lua_isnumber(L, valIndex))
        {
            lua_Number num = lua_tonumber(L, valIndex);
            newValueUd->mDynamicAny = daud->mDynamicAny + num;
            rv = 1;
        }
        else if (lua_isstring(L, valIndex))
        {
            const char* str = lua_tostring(L, valIndex);
            newValueUd->mDynamicAny = daud->mDynamicAny + str;
            rv = 1;
        }
        else
            luaL_error(L, "invalid type supplied to __add: %s", lua_typename(L, lua_type(L, valIndex)));
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

int DynamicAnyUserdata::metamethod__sub(lua_State* L)
{
    int rv = 0;
    int udIndex = 1;
    int valIndex = 2;
    
    DynamicAnyUserdata* daud;
    
    if (!lua_isuserdata(L, 1))
    {
        udIndex = 2;
        valIndex = 1;
    }
    
    daud = reinterpret_cast<DynamicAnyUserdata*>(luaL_checkudata(L, udIndex, "Poco.DynamicAny.metatable"));
    
    try
    {
        void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
        luaL_getmetatable(L, "Poco.DynamicAny.metatable");
        lua_setmetatable(L, -2);
        DynamicAnyUserdata* newValueUd = new(ud) DynamicAnyUserdata(Poco::DynamicAny());
        
        if (lua_isuserdata(L, valIndex))
        {
            lua_getmetatable(L, udIndex);
            lua_getmetatable(L, valIndex);
            int equal = lua_rawequal(L, -1, -2);
            lua_pop(L, 2);
            
            if (equal)
            {
                DynamicAnyUserdata* daudOther = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, valIndex));
                if (udIndex == 1)
                    newValueUd->mDynamicAny = daud->mDynamicAny - daudOther->mDynamicAny;
                else
                    newValueUd->mDynamicAny = daudOther->mDynamicAny - daud->mDynamicAny;
                rv = 1;
            }
            else
                luaL_error(L, "expected two Poco.DynamicAny userdata");
        }
        else if (lua_isnumber(L, valIndex))
        {
            lua_Number num = lua_tonumber(L, valIndex);
            
            if (udIndex == 1)
                newValueUd->mDynamicAny = daud->mDynamicAny - num;
            else
                newValueUd->mDynamicAny = num - daud->mDynamicAny;
            
            rv = 1;
        }
        else
            luaL_error(L, "invalid type supplied to __sub: %s", lua_typename(L, lua_type(L, valIndex)));
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

int DynamicAnyUserdata::metamethod__mul(lua_State* L)
{
    int rv = 0;
    int udIndex = 1;
    int valIndex = 2;
    
    DynamicAnyUserdata* daud;
    
    if (!lua_isuserdata(L, 1))
    {
        udIndex = 2;
        valIndex = 1;
    }
    
    daud = reinterpret_cast<DynamicAnyUserdata*>(luaL_checkudata(L, udIndex, "Poco.DynamicAny.metatable"));
    
    try
    {
        void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
        luaL_getmetatable(L, "Poco.DynamicAny.metatable");
        lua_setmetatable(L, -2);
        DynamicAnyUserdata* newValueUd = new(ud) DynamicAnyUserdata(Poco::DynamicAny());
        
        if (lua_isuserdata(L, valIndex))
        {
            lua_getmetatable(L, udIndex);
            lua_getmetatable(L, valIndex);
            int equal = lua_rawequal(L, -1, -2);
            lua_pop(L, 2);
            
            if (equal)
            {
                DynamicAnyUserdata* daudOther = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, valIndex));
                newValueUd->mDynamicAny = daud->mDynamicAny * daudOther->mDynamicAny;
                rv = 1;
            }
            else
                luaL_error(L, "expected two Poco.DynamicAny userdata");
        }
        else if (lua_isnumber(L, valIndex))
        {
            lua_Number num = lua_tonumber(L, valIndex);
            newValueUd->mDynamicAny = daud->mDynamicAny * num;
            rv = 1;
        }
        else
            luaL_error(L, "invalid type supplied to __mul: %s", lua_typename(L, lua_type(L, valIndex)));
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

int DynamicAnyUserdata::metamethod__div(lua_State* L)
{
    int rv = 0;
    int udIndex = 1;
    int valIndex = 2;
    
    DynamicAnyUserdata* daud;
    
    if (!lua_isuserdata(L, 1))
    {
        udIndex = 2;
        valIndex = 1;
    }
    
    daud = reinterpret_cast<DynamicAnyUserdata*>(luaL_checkudata(L, udIndex, "Poco.DynamicAny.metatable"));
    
    try
    {
        void* ud = lua_newuserdata(L, sizeof (DynamicAnyUserdata));
        luaL_getmetatable(L, "Poco.DynamicAny.metatable");
        lua_setmetatable(L, -2);
        DynamicAnyUserdata* newValueUd = new(ud) DynamicAnyUserdata(Poco::DynamicAny());
        
        if (lua_isuserdata(L, valIndex))
        {
            lua_getmetatable(L, udIndex);
            lua_getmetatable(L, valIndex);
            int equal = lua_rawequal(L, -1, -2);
            lua_pop(L, 2);
            
            if (equal)
            {
                DynamicAnyUserdata* daudOther = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, valIndex));
                
                if (udIndex == 1)
                    newValueUd->mDynamicAny = daud->mDynamicAny / daudOther->mDynamicAny;
                else
                    newValueUd->mDynamicAny = daudOther->mDynamicAny / daud->mDynamicAny;
                
                rv = 1;
            }
            else
                luaL_error(L, "expected two Poco.DynamicAny userdata");
        }
        else if (lua_isnumber(L, valIndex))
        {
            lua_Number num = lua_tonumber(L, valIndex);
            if (udIndex == 1)
                newValueUd->mDynamicAny = daud->mDynamicAny / num;
            else
                newValueUd->mDynamicAny = num / daud->mDynamicAny;
            rv = 1;
        }
        else
            luaL_error(L, "invalid type supplied to __div: %s", lua_typename(L, lua_type(L, valIndex)));
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

int DynamicAnyUserdata::metamethod__eq(lua_State* L)
{
    int rv = 0;
    int udIndex = 1;
    int valIndex = 2;
    
    DynamicAnyUserdata* daud;
    
    if (!lua_isuserdata(L, 1))
    {
        udIndex = 2;
        valIndex = 1;
    }
    
    daud = reinterpret_cast<DynamicAnyUserdata*>(luaL_checkudata(L, udIndex, "Poco.DynamicAny.metatable"));
    
    try
    {
        if (lua_isuserdata(L, valIndex))
        {
            lua_getmetatable(L, udIndex);
            lua_getmetatable(L, valIndex);
            int equal = lua_rawequal(L, -1, -2);
            lua_pop(L, 2);
            
            if (equal)
            {
                DynamicAnyUserdata* daudOther = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, valIndex));
                bool result = daud->mDynamicAny == daudOther->mDynamicAny;
                lua_pushboolean(L, result);
                rv = 1;
            }
            else
                luaL_error(L, "expected two Poco.DynamicAny userdata");
        }
        else
            luaL_error(L, "invalid type supplied to __eq: %s", lua_typename(L, lua_type(L, valIndex)));
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

int DynamicAnyUserdata::metamethod__lt(lua_State* L)
{
    int rv = 0;
    int udIndex = 1;
    int valIndex = 2;
    
    DynamicAnyUserdata* daud;
    
    if (!lua_isuserdata(L, 1))
    {
        udIndex = 2;
        valIndex = 1;
    }
    
    daud = reinterpret_cast<DynamicAnyUserdata*>(luaL_checkudata(L, udIndex, "Poco.DynamicAny.metatable"));
    
    try
    {
        if (lua_isuserdata(L, valIndex))
        {
            lua_getmetatable(L, udIndex);
            lua_getmetatable(L, valIndex);
            int equal = lua_rawequal(L, -1, -2);
            lua_pop(L, 2);
            
            if (equal)
            {
                DynamicAnyUserdata* daudOther = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, valIndex));
                bool result;
                
                if (udIndex == 1)
                    result = daud->mDynamicAny < daudOther->mDynamicAny;
                else
                    result = daudOther->mDynamicAny < daud->mDynamicAny;
                
                lua_pushboolean(L, result);
                rv = 1;
            }
            else
                luaL_error(L, "expected two Poco.DynamicAny userdata");
        }
        else
            luaL_error(L, "invalid type supplied to __lt: %s", lua_typename(L, lua_type(L, valIndex)));
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

int DynamicAnyUserdata::metamethod__le(lua_State* L)
{
    int rv = 0;
    int udIndex = 1;
    int valIndex = 2;
    
    DynamicAnyUserdata* daud;
    
    if (!lua_isuserdata(L, 1))
    {
        udIndex = 2;
        valIndex = 1;
    }
    
    daud = reinterpret_cast<DynamicAnyUserdata*>(luaL_checkudata(L, udIndex, "Poco.DynamicAny.metatable"));
    
    try
    {
        if (lua_isuserdata(L, valIndex))
        {
            lua_getmetatable(L, udIndex);
            lua_getmetatable(L, valIndex);
            int equal = lua_rawequal(L, -1, -2);
            lua_pop(L, 2);
            
            if (equal)
            {
                DynamicAnyUserdata* daudOther = reinterpret_cast<DynamicAnyUserdata*>(lua_touserdata(L, valIndex));
                bool result;
                
                if (udIndex == 1)
                    result = daud->mDynamicAny <= daudOther->mDynamicAny;
                else
                    result = daudOther->mDynamicAny <= daud->mDynamicAny;
                
                lua_pushboolean(L, result);
                rv = 1;
            }
            else
                luaL_error(L, "expected two Poco.DynamicAny userdata");
        }
        else
            luaL_error(L, "invalid type supplied to __le: %s", lua_typename(L, lua_type(L, valIndex)));
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
