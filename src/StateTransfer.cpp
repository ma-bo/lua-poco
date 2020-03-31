#include "StateTransfer.h"
#include "Userdata.h"
#include <cstring>
#include <cstdlib>
#include <exception>

namespace LuaPoco
{

const char* STATE_TRANSFER_SOURCE_TABLES = "Poco.StateTransfer.Source.Tables";
const char* STATE_TRANSFER_SOURCE_TABLES_LASTINDEX = "Poco.StateTransfer.Source.LastIndex";
const char* STATE_TRANSFER_DESTINATION_TABLES = "Poco.StateTransfer.Destination.Tables";

#define DEFAULT_TRANSFER_BUFF_SIZE 1000

TransferBuffer::TransferBuffer() :
    mBuffer(NULL), mReadIdx(0), mWriteIdx(0)
{
    mCapacity = DEFAULT_TRANSFER_BUFF_SIZE;
    mBuffer = static_cast<char*>(std::malloc(mCapacity));
    if (!mBuffer) throw std::bad_alloc();
}

TransferBuffer::TransferBuffer(size_t startSize) :
    mBuffer(NULL), mReadIdx(0), mWriteIdx(0)
{
    mCapacity = startSize > 0 ? startSize : DEFAULT_TRANSFER_BUFF_SIZE;
    mBuffer = static_cast<char*>(std::malloc(mCapacity));
    if (!mBuffer) throw std::bad_alloc();
}

TransferBuffer::~TransferBuffer()
{
    std::free(mBuffer);
    mBuffer = NULL;
}

void TransferBuffer::insert(const char* data, size_t amount)
{
    if (amount > mCapacity - mWriteIdx)
    {
        mCapacity += amount + 1000;
        
        char* tempBuffer = static_cast<char*>(std::realloc(mBuffer, mCapacity));
        if (tempBuffer) mBuffer = tempBuffer;
        else throw std::bad_alloc();
    }
    
    std::memcpy(mBuffer + mWriteIdx, data, amount);
    mWriteIdx += amount;
}

void TransferBuffer::get(const char*& data, size_t& amount)
{
    size_t available = mWriteIdx - mReadIdx;
    
    amount = available;
    if (available == 0) data = NULL;
    else data = mBuffer + mReadIdx;
    
    mReadIdx += amount;
}

size_t TransferBuffer::contentSize()
{
    return mWriteIdx;
}

int functionWriter(lua_State* L, const void* p, size_t sz, void* ud)
{
    int rv = 0;
    
    TransferBuffer* tb = static_cast<TransferBuffer*>(ud);
    try
    {
        tb->insert(static_cast<const char*>(p), sz);
    }
    catch (const std::exception& e)
    {
        (void) e;
        // only likely scenario is a std::bad_alloc
        rv = -1;
    }
    
    return rv;
}

const char* functionReader(lua_State* L, void* data, size_t* size)
{
    const char* rv = NULL;
    TransferBuffer* tb = static_cast<TransferBuffer*>(data);
    tb->get(rv, *size);
    
    return rv;
}

bool transferFunction(lua_State* toL, lua_State* fromL)
{
    bool result = false;
    if (!lua_iscfunction(fromL, -1) && lua_isfunction(fromL, -1))
    {
        try
        {
            TransferBuffer tb;
            
            #if LUA_VERSION_NUM > 502
            if (lua_dump(fromL, functionWriter, &tb, 0) == 0)
            #else
            if (lua_dump(fromL, functionWriter, &tb) == 0)
            #endif
            {
                #if LUA_VERSION_NUM > 501
                if (lua_load(toL, functionReader, &tb, "transferFunction", NULL) == 0)
                    result = true;
                #else
                if (lua_load(toL, functionReader, &tb, "transferFunction") == 0)
                    result = true;
                #endif
            }
        }
        catch (const std::exception& e)
        {
            (void) e;
            // catch a std::bad_alloc
            result = false;
        }
    }
    
    return result;
}

// creates a table on toL, and stores it in REGISTRY[STATE_TRANSFER_DESTINATION_TABLES].
// the source table at the top of fromL, is stored in REGISTRY[STATE_TRANSFER_SOURCE_TABLES]
// populateTables() will later take each source and destination values from the registry and use
// populateTableValues() to transfer individual values.
//
// the rationale is to create an iterative way to populate nested tables across states, rather than
// a recursive approach which is constrained by the call stack size.
bool transferTable(lua_State* toL, lua_State* fromL)
{
    bool result = false;

    // create new table, which will be left on stack when done.
    lua_newtable(toL);
  
    // fetch the transfer tables.
    lua_getfield(toL, LUA_REGISTRYINDEX, STATE_TRANSFER_DESTINATION_TABLES);
    lua_getfield(fromL, LUA_REGISTRYINDEX, STATE_TRANSFER_SOURCE_TABLES);

    // these transfer tables are required to be present, but check for safety sake anyway.
    if (lua_istable(fromL, -1) && lua_istable(toL, -1))
    {
        // fetch the lastindex where a table is stored, 0/nil means no tables stored.
        lua_getfield(fromL, -1, STATE_TRANSFER_SOURCE_TABLES_LASTINDEX);
        lua_Integer lastIndex = lua_tointeger(fromL, -1);
        lua_pop(fromL, 1);
        ++lastIndex;

        // duplicate the tables to be transferred and set them in the respective transfer table.
        lua_pushvalue(fromL, -2);
        lua_rawseti(fromL, -2, lastIndex);

        lua_pushvalue(toL, -2);
        lua_rawseti(toL, -2, lastIndex);

        lua_pushinteger(fromL, lastIndex);
        lua_setfield(fromL, -2, STATE_TRANSFER_SOURCE_TABLES_LASTINDEX);

        result = true;
    }

    // remove transfer tables from stack, leaving the orignals that were transferred.
    lua_pop(fromL, 1);
    lua_pop(toL, 1);

    return result;
}

bool transferValueInternal(lua_State* toL, lua_State* fromL)
{
    bool result = false;
    int type = lua_type(fromL, -1);
    
    switch (type)
    {
    case LUA_TNIL:
        lua_pushnil(toL);
        result = true;
        break;
    case LUA_TNUMBER:
        lua_pushnumber(toL, lua_tonumber(fromL, -1));
        result = true;
        break;
    case LUA_TBOOLEAN:
        lua_pushboolean(toL, lua_toboolean(fromL, -1));
        result = true;
        break;
    case LUA_TSTRING:
    {
        size_t len;
        const char* str = lua_tolstring(fromL, -1, &len);
        lua_pushlstring(toL, str, len);
        result = true;
        break;
    }
    case LUA_TTABLE:
    {
        result = transferTable(toL, fromL);
        break;
    }
    case LUA_TFUNCTION:
        result = transferFunction(toL, fromL);
        break;
    case LUA_TUSERDATA:
    {
        Userdata* ud = getPrivateUserdata(fromL, -1);
        if (ud)
            result = ud->copyToState(toL);
        break;
    }
    case LUA_TTHREAD:
        // copying coroutines is not supported.
        break;
    case LUA_TLIGHTUSERDATA:
        lua_pushlightuserdata(toL, lua_touserdata(fromL, -1));
        result = true;
        break;
    default:
        break;
    }
    
    return result;
}

// destination table, and source table should be at -1 index.
bool transferTableValues(lua_State* toL, lua_State* fromL)
{
    bool result = true;

    lua_pushnil(fromL);

    // iterate key/value pairs
    // transfer them to the target state
    // set them in the destination table.
    while (lua_next(fromL, -2))
    {
        // duplicate key so it can be transferred first.
        lua_pushvalue(fromL, -2);

        // transfer key
        if (transferValueInternal(toL, fromL))
        {
            // pop extra key
            lua_pop(fromL, 1);
            // transfer value
            if (transferValueInternal(toL, fromL))
            {
                // pop value
                lua_pop(fromL, 1);
                // set the transfered value, key, leaving just the table.
                lua_rawset(toL, -3);
            }
            else
            {
                result = false;
                break;
            }
        }
        else
        {
            result = false;
            break;
        }
    }
    
    return result;
}

bool populateTables(lua_State* toL, lua_State* fromL)
{
    bool result = true;
    int fromTop = lua_gettop(fromL);
    int toTop = lua_gettop(toL);

    lua_getfield(fromL, LUA_REGISTRYINDEX, STATE_TRANSFER_SOURCE_TABLES);
    lua_getfield(toL, LUA_REGISTRYINDEX, STATE_TRANSFER_DESTINATION_TABLES);
    
    if (lua_istable(fromL, -1) && lua_istable(toL, -1))
    {
        while (true)
        {
            // lastIndex = registry.source_tables.lastindex
            lua_getfield(fromL, -1, STATE_TRANSFER_SOURCE_TABLES_LASTINDEX);
            lua_Integer lastIndex = lua_tointeger(fromL, -1);
            lua_pop(fromL, 1);

            // check if there is work to do, if not return.
            if (lastIndex == 0) { break; }

            // fetch the last added source/destination table pair.
            lua_rawgeti(fromL, -1, lastIndex);
            lua_rawgeti(toL, -1, lastIndex);

            // remove these tables from the in-progress transfer table list.
            // this frees up this slot and prevents "holes" in the table for any upcoming
            // nested tables.
            lua_pushnil(fromL);
            lua_rawseti(fromL, -3, lastIndex);
            lua_pushnil(toL);
            lua_rawseti(toL, -3, lastIndex);
            
            // reset the last index value
            lua_pushinteger(fromL, --lastIndex);
            lua_setfield(fromL, -3, STATE_TRANSFER_SOURCE_TABLES_LASTINDEX);

            if (!transferTableValues(toL, fromL))
            {
                result = false;
                break;
            }
            
            // source key/value pairs have been copied to destination table.
            // pop source/destination from stack.
            lua_pop(fromL, 1);
            lua_pop(toL, 1);
        }
    }

    lua_settop(fromL, fromTop);
    lua_settop(toL, toTop);

    return result;
}

bool transferValue(lua_State* toL, lua_State* fromL)
{
    bool result = false;
    bool isTableTransfer = lua_istable(fromL, -1);

    // prepare transfer tables only if we're operating on a table
    if (isTableTransfer)
    {
        lua_newtable(fromL);
        lua_setfield(fromL, LUA_REGISTRYINDEX, STATE_TRANSFER_SOURCE_TABLES);
        lua_newtable(toL);
        lua_setfield(toL, LUA_REGISTRYINDEX, STATE_TRANSFER_DESTINATION_TABLES);
    }

    result = transferValueInternal(toL, fromL);

    // remove transfer tables in registry
    if (isTableTransfer)
    {
        // do not attempt populating the tables if table creation failed.
        if (result) { result = populateTables(toL, fromL); }
        
        lua_pushnil(fromL);
        lua_setfield(fromL, LUA_REGISTRYINDEX, STATE_TRANSFER_SOURCE_TABLES);
        lua_pushnil(toL);
        lua_setfield(toL, LUA_REGISTRYINDEX, STATE_TRANSFER_DESTINATION_TABLES);
    }
    
    return result;
}

} // LuaPoco
