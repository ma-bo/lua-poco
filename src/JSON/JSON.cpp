/// JSON encoding and decoding
// Encoding Lua tables to JSON strings, and decoding JSON strings to Lua tables.
// @module json

#include "JSON.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Handler.h>
#include <Poco/JSON/JSONException.h>
#include <Poco/SharedPtr.h>
#include "Userdata.h"
#include "LuaPocoUtils.h"

int luaopen_poco_json(lua_State* L)
{
    struct LuaPoco::UserdataMethod methods[] = 
    {
        { "encode", LuaPoco::JSON::encode },
        { "decode", LuaPoco::JSON::decode },
        { "emptyObject", LuaPoco::JSON::getEmptyObject },
        { "emptyArray", LuaPoco::JSON::getEmptyArray },
        { NULL, NULL}
    };
    
    lua_createtable(L, 0, 2);
    setMetatableFunctions(L, methods);
   
    return 1;
}

namespace LuaPoco
{

char jsonNull[] = "Poco.JSON.null";
char jsonEmptyArray[] = "Poco.JSON.Empty.Array";
char jsonEmptyObject[] = "Poco.JSON.Empty.Object";

struct TableInfo
{
    enum TableType { object, array} tableType;
    size_t currentArrayIndex;
    int tableStackIndex;
};

class LuaHandler : public Poco::JSON::Handler
{
public:

    LuaHandler(lua_State* L) : mState(L), mBaseTop(lua_gettop(L)) {}
    virtual ~LuaHandler() {}

    virtual void reset()
    {
        lua_settop(mState, mBaseTop);
    }

    virtual void startObject()
    {
        lua_newtable(mState);
        TableInfo ti = { TableInfo::TableType::object, 0, lua_gettop(mState) };
        mTableQueue.push_back(ti);
    }

    virtual void endObject()
    {
        mTableQueue.pop_back();
        setValue();
    }

    virtual void startArray()
    {
        lua_newtable(mState);
        TableInfo ti = { TableInfo::TableType::array, 0, lua_gettop(mState) };
        mTableQueue.push_back(ti);
    }

    virtual void endArray()
    {
        mTableQueue.pop_back();
        setValue();
    }

    virtual void key(const std::string& k)
    {
        // leave the key on the stack such that on a value, setValue is called to pop both off.
        lua_pushlstring(mState, k.c_str(), k.size());
    }

    virtual void null()
    {
        // use sentinel value jsonNull 
        lua_pushlightuserdata(mState, static_cast<void*>(jsonNull));
        setValue();
    }

    // based on the poco code, int and unsigned are pure virtual functions, but will never be
    // called as ParserImpl only uses Int64s for whatever reason.  this will make them work
    // regardless, if for some reason they're turned on in the future.
    virtual void value(int v)
    {
        lua_Integer i = 0;
        if (checkSignedToLuaInteger<int>(v, i))
        {
            lua_pushinteger(mState, static_cast<lua_Integer>(i));
            setValue();
        }
        else throw Poco::JSON::JSONException("out of range number");
    }

    virtual void value(unsigned v)
    {
        lua_Integer i = 0;
        if (checkUnsignedToLuaInteger<unsigned>(v, i))
        {
            lua_pushinteger(mState, static_cast<lua_Integer>(i));
            setValue();
        }
        else throw Poco::JSON::JSONException("out of range number");
    }

#if defined(POCO_HAVE_INT64)
    virtual void value(Poco::Int64 v)
    {
        lua_Integer i = 0;
        if (checkSignedToLuaInteger<Poco::Int64>(v, i))
        {
            lua_pushinteger(mState, static_cast<lua_Integer>(i));
            setValue();
        }
        else throw Poco::JSON::JSONException("out of range number");
    }

    virtual void value(Poco::UInt64 v)
    {
        lua_Integer i = 0;
        if (checkUnsignedToLuaInteger<Poco::UInt64>(v, i))
        {
            lua_pushinteger(mState, static_cast<lua_Integer>(i));
            setValue();
        }
        else throw Poco::JSON::JSONException("out of range number");
    }
#endif

    virtual void value(const std::string& s)
    {
        lua_pushlstring(mState, s.c_str(), s.size());
        setValue();
    }

    virtual void value(double d)
    {
        lua_pushnumber(mState, static_cast<lua_Number>(d));
        setValue();
    }

    virtual void value(bool b)
    {
        lua_pushboolean(mState, static_cast<int>(b));
        setValue();
    }

private:
    // utility functions
    void setValue()
    {
        if (mTableQueue.size() > 0)
        {
            TableInfo& ti = mTableQueue.back();
            if (ti.tableType == TableInfo::TableType::array)
            {
                ++ti.currentArrayIndex;
                lua_rawseti(mState, ti.tableStackIndex, ti.currentArrayIndex);
            }
            else
            {
                const char *tn = lua_typename(mState, lua_type(mState, -1));
                lua_rawset(mState, ti.tableStackIndex);
            }
        }
        else
        {
            // the last value is a table that should be left behind and returned.
            if (lua_type(mState, -1) != LUA_TTABLE) 
                throw Poco::JSON::JSONException("attempt to set a value when no objects present");
        }
    }

    lua_State* mState;
    int mBaseTop;
    
    std::vector<TableInfo> mTableQueue;
};

/// encodes a table into a JSON string.
// @table table to encode
// @return value as string or nil. (error)
// @return error message.
// @function encode
int JSON::encode(lua_State* L)
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

/// decodes a JSON string into a table.
// @string JSON encoded string
// @return table or nil. (error)
// @return error message.
// @function decode
int JSON::decode(lua_State* L)
{
    int rv = 0;
    const char* jss = luaL_checkstring(L, 1);
    
    try
    {
        Poco::SharedPtr<LuaHandler> lh(new LuaHandler(L));
        Poco::JSON::Parser jsonParser(lh);
        jsonParser.parse(jss);

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

/// returns the 'null' sentinel value as a lightuserdata.
// @return lightuserdata
// @function null
int JSON::getNull(lua_State* L)
{
    lua_pushlightuserdata(L, static_cast<void*>(jsonNull));
    return 1;
}

/// returns the 'emptyObject' sentinel value as a lightuserdata.
// @return lightuserdata
// @function emptyObject
int JSON::getEmptyObject(lua_State* L)
{
    lua_pushlightuserdata(L, static_cast<void*>(jsonEmptyObject));
    return 1;
}

/// returns the 'emptyArray' sentinel value as a lightuserdata.
// @return lightuserdata
// @function emptyArray
int JSON::getEmptyArray(lua_State* L)
{
    lua_pushlightuserdata(L, static_cast<void*>(jsonEmptyArray));
    return 1;
}

} // LuaPoco
