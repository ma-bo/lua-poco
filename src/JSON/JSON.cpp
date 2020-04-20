/// JSON encoding and decoding
// Encoding Lua tables to JSON strings, and decoding JSON strings to Lua tables.
// @module json

#include "JSON.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Handler.h>
#include <Poco/JSON/PrintHandler.h>
#include <Poco/JSON/JSONException.h>
#include <Poco/JSONString.h>
#include <Poco/SharedPtr.h>
#include "Userdata.h"
#include "LuaPocoUtils.h"

int luaopen_poco_json(lua_State* L)
{
    struct LuaPoco::UserdataMethod methods[] =
    {
        { "encode", LuaPoco::JSON::encode },
        { "decode", LuaPoco::JSON::decode },
        { "null", LuaPoco::JSON::getNull },
        { "emptyObject", LuaPoco::JSON::getEmptyObject },
        { "emptyArray", LuaPoco::JSON::getEmptyArray },
        { NULL, NULL}
    };

    lua_createtable(L, 0, 5);
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

class JsonDecoder : public Poco::JSON::Handler
{
public:

    JsonDecoder(lua_State* L) : mState(L), mBaseTop(lua_gettop(L)) {}
    virtual ~JsonDecoder() {}

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
    // called as ParserImpl only uses Int64s for whatever reason.  including for completeness.
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

class TableEncoder
{
public:
    TableEncoder(lua_State* L, unsigned indent) : mState(L), mStream(), mPrintHandler(mStream, indent) {}
    ~TableEncoder() {}

    bool encode()
    {
        handleTable();

        while (mTableQueue.size() > 0)
        {
            TableInfo& ti = mTableQueue.back();
            if (ti.tableType == TableInfo::TableType::array)
            {
                lua_rawgeti(mState, ti.tableStackIndex, ++ti.currentArrayIndex);
                if (!lua_isnil(mState, -1)) { handleValue(); }
                else
                {
                    // pop nil, array
                    lua_pop(mState, 2);
                    mTableQueue.pop_back();
                    mPrintHandler.endArray();
                }
            }
            else
            {
                // handleTable() leaves nil on the stack to for the nested table iteration.
                if (lua_next(mState, ti.tableStackIndex))
                {
                    if (lua_isstring(mState, -2))
                    {
                        const char* key = lua_tostring(mState, -2);
                        mPrintHandler.key(key);
                        handleValue();
                    }
                    else throw Poco::JSON::JSONException("encountered key value that is not a string.");
                }
                else
                {
                    // done with object, pop it.
                    lua_pop(mState, 1);
                    mTableQueue.pop_back();
                    mPrintHandler.endObject();
                }
            }
        }

        std::string jsonString = mStream.str();
        lua_pushlstring(mState, jsonString.c_str(), jsonString.size());
        // return all pending tables were processed
        return mTableQueue.empty();
    }

private:
    // arrays are defined as tables that operate as sequences from 1 to n with [n + 1] == nil to terminate.
    // objects are defined as not having an array part, and only string keys being present.
    bool isArray(int index)
    {
        lua_rawgeti(mState, index, 1);
        bool result = !lua_isnil(mState, -1);
        lua_pop(mState, 1);
        return result;
    }

    void handleTable()
    {
        // store table information in mQueue.
        TableInfo ti =
        {
            isArray(-1) ? TableInfo::TableType::array : TableInfo::TableType::object,
            0,
            lua_gettop(mState)
        };
        mTableQueue.push_back(ti);

        // leave key on the top of the stack so the loop can process it with calls to lua_next.
        // inform printer of the start of a new object/array.
        if (ti.tableType == TableInfo::TableType::object)
        {
            mPrintHandler.startObject();
            lua_pushnil(mState);
        }
        else { mPrintHandler.startArray(); }
    }

    void handleValue()
    {
        int type = lua_type(mState, -1);

        switch (type)
        {
        case LUA_TNIL:
            throw Poco::JSON::JSONException("nil is an invalid value, use json.null");
            break;
        case LUA_TNUMBER:
        #if LUA_VERSION_NUM > 502
            if (lua_isinteger(mState, -1))
            {
                lua_Integer i = lua_tointeger(mState, -1);
                mPrintHandler.value(i);
            }
            else
        #endif
            {
                lua_Number n = lua_tonumber(mState, -1);
                mPrintHandler.value(n);
            }
            break;
        case LUA_TBOOLEAN:
        {
            bool b = static_cast<bool>(lua_toboolean(mState, -1));
            mPrintHandler.value(b);
            break;
        }
        case LUA_TSTRING:
        {
            const std::string str = lua_tostring(mState, -1);
            mPrintHandler.value(str);
            break;
        }
        case LUA_TTABLE:
            handleTable();
            // the table value must stay on the top of the stack in order to be iterated by
            // encode loop.  when the end of the table is encountered, it is popped.
            // returning here avoids the pop that is needed for all other values.
            return;
            break;
        case LUA_TFUNCTION:
            throw Poco::JSON::JSONException("function type invalid for json.");
            break;
        case LUA_TUSERDATA:
            throw Poco::JSON::JSONException("userdata type invalid for json.");
            break;
        case LUA_TTHREAD:
            throw Poco::JSON::JSONException("thread type invalid for json.");
            break;
        case LUA_TLIGHTUSERDATA:
        {
            const char *lud = static_cast<const char*>(lua_touserdata(mState, -1));

            if (lud == jsonNull) { mPrintHandler.null(); }
            else if (lud == jsonEmptyArray) { mPrintHandler.startArray(); mPrintHandler.endArray(); }
            else if (lud == jsonEmptyObject) { mPrintHandler.startObject(); mPrintHandler.endObject(); }
            else throw Poco::JSON::JSONException("unknown json lightuserdata value.");
            break;
        }
        default:
            throw Poco::JSON::JSONException("unknown value for json conversion.");
            break;
        }

        // all values except for table needs to be popped.
        // the table case returns early to avoid this pop.
        lua_pop(mState, 1);
    }

    lua_State* mState;
    std::stringstream mStream;
    Poco::JSON::PrintHandler mPrintHandler;
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
    unsigned indent = 0;

    luaL_checktype(L, 1, LUA_TTABLE);
    if (lua_isnumber(L, 2)) { indent = static_cast<unsigned>(lua_tointeger(L, 2)); }
    // TableEncoder expects the table it is encoding at the top of the stack.
    lua_pushvalue(L, 1);

    try
    {
        TableEncoder te(L, indent);
        // either a string is returned at the top of the stack
        // or nil, errmsg
        if (te.encode()) { rv = 1; }
        else { rv = 2; }
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
        Poco::SharedPtr<JsonDecoder> lh(new JsonDecoder(L));
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
