#include "StateTransfer.h"
#include "Userdata.h"
#include <cstring>
#include <exception>

namespace LuaPoco
{

TransferBuffer::TransferBuffer() :
    Buffer<char>(1000), mWriteIdx(0)
{
}

TransferBuffer::TransferBuffer(size_t startSize) :
    Buffer<char>(startSize), mWriteIdx(0)
{
}

TransferBuffer::~TransferBuffer()
{
}

void TransferBuffer::insert(const char* data, size_t amount)
{
    if (amount > size() - mWriteIdx)
        resize(size() + amount + 1000, true);
    
    std::memcpy(begin() + mWriteIdx, data, amount);
    mWriteIdx += amount;
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
    
    rv = tb->begin();
    *size = tb->size();
    
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
            if (lua_dump(fromL, functionWriter, &tb) == 0)
            {
                // sigh, was hoping to do this without any ifdefs... 
                // alas, i don't see how this one can be avoided
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

bool transferTable(lua_State* toL, lua_State* fromL)
{
    bool result = false;
    size_t nestLevel = 1;
    int fromEntryTop = lua_gettop(fromL);
    int toEntryTop = lua_gettop(toL);
    
    if (!lua_istable(fromL, -1))
        return false;
    
    lua_newtable(toL);
    
    lua_pushnil(fromL);
    // get next element from table
    while (nestLevel > 0)
    {
        if (lua_next(fromL, -2) != 0)
        {
            if (lua_istable(fromL, -1))
            {
                // create the new table first as we will need to potentially 
                // write values to it in the new state later
                lua_newtable(toL);
                // push copy of key to fromTop, such that it can be transfered
                lua_pushvalue(fromL, -2);
                if (!transferValue(toL, fromL))
                    break;
                // pop the key off leaving fromL: -1 = sub table value, -2 = key, -3 = table
                lua_pop(fromL, 1);
                
                // copy the newly created table and set the key/newtable pair as a 
                // value into the parent table
                lua_pushvalue(toL, -2);
                // set table leaving us with -1 = newtable, -2 = parent table
                lua_settable(toL, -4);
                
                // iterate this child table
                ++nestLevel;
                lua_pushnil(fromL);
            }
            else
            {
                // push copy of key to top, such that it can be transfered
                lua_pushvalue(fromL, -2);
                if (!transferValue(toL, fromL))
                    break;
                // pop the key off leaving -1 = value, -2 = key, -3 = table
                lua_pop(fromL, 1);
                // transfer the value
                if (!transferValue(toL, fromL))
                    break;
                // settable in the remote state
                lua_settable(toL, -3);
                // pop value from top, leaving key for the call to lua_next()
                lua_pop(fromL, 1);
            }
        }
        else
        {
            // pop the child table off both stacks leaving the just the parent on
            // the to state, and key/table on the fromState (for iteration)
            // except on the last pass, as we want to keep the original tables in place
            if (nestLevel > 1)
            {
                lua_pop(fromL, 1);
                lua_pop(toL, 1);
            }
            --nestLevel;
        }
    }
    
    if (nestLevel == 0)
        result = true;
    
    // clean up any residual stuff on both states in the case of a failure
    lua_pop(fromL, fromEntryTop - lua_gettop(fromL));
    lua_pop(fromL, toEntryTop - lua_gettop(toL));
    
    return result;
}

bool transferValue(lua_State* toL, lua_State* fromL)
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
        result = transferTable(toL, fromL);
        break;
    case LUA_TFUNCTION:
        result = transferFunction(toL, fromL);
        break;
    case LUA_TUSERDATA:
    {
        Userdata* ud = getPrivateUserdata(fromL, -1);
        if (ud && ud->copyToState(toL))
            result = true;
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

} // LuaPoco
