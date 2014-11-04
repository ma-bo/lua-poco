#include "LuaPoco.h"
#include "foundation/File.h"
#include "foundation/RegularExpression.h"
#include "foundation/DynamicAny.h"
#include "foundation/Timestamp.h"
#include "foundation/Checksum.h"
#include "foundation/Environment.h"
#include "foundation/Pipe.h"
#include "foundation/NamedEvent.h"
#include "foundation/NamedMutex.h"
#include "foundation/ProcessHandle.h"
#include "foundation/Process.h"
#include "foundation/Semaphore.h"
#include "foundation/FastMutex.h"
#include "foundation/Mutex.h"
#include "foundation/Thread.h"
#include "foundation/Event.h"
#include "foundation/IStream.h"
#include "foundation/OStream.h"
#include "foundation/PipeOStream.h"
#include "foundation/PipeIStream.h"

namespace LuaPoco
{

// create all metatables per class
// and load class constructors into the poco table.
bool loadMetatables(lua_State* L)
{
    bool rv = false;
    lua_getfield(L, LUA_REGISTRYINDEX, "poco.metatables.registered");
    if (lua_isnil(L, -1))
    {
        if (
            FileUserdata::registerFile(L) &&
            RegularExpressionUserdata::registerRegularExpression(L) &&
            DynamicAnyUserdata::registerDynamicAny(L) &&
            TimestampUserdata::registerTimestamp(L) &&
            ChecksumUserdata::registerChecksum(L) &&
            PipeUserdata::registerPipe(L) &&
            NamedEventUserdata::registerNamedEvent(L) && 
            NamedMutexUserdata::registerNamedMutex(L) &&
            ProcessHandleUserdata::registerProcessHandle(L) &&
            SemaphoreUserdata::registerSemaphore(L) &&
            FastMutexUserdata::registerFastMutex(L) &&
            MutexUserdata::registerMutex(L) &&
            ThreadUserdata::registerThread(L) &&
            EventUserdata::registerEvent(L) &&
            PipeOStreamUserdata::registerPipeOStream(L) && 
            PipeIStreamUserdata::registerPipeIStream(L) && 
            IStreamUserdata::registerIStream(L) &&
            OStreamUserdata::registerOStream(L)
        )
        {
            setupPrivateUserdata(L);
            rv = true;
        }
    }
    else
        rv = true;
    
    lua_pop(L, 1);
    
    return rv;
}

int loadConstructor(lua_State*L, lua_CFunction cons)
{
    int rv = 0;
    
    if (LuaPoco::loadMetatables(L))
    {
        lua_createtable(L, 0, 3);
        lua_pushcfunction(L, cons);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, "new");
        lua_setfield(L, -2, "__call");
        lua_pushvalue(L, -1);
        lua_setmetatable(L, -2);
        rv = 1;
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, "failed to create required poco metatables");
        rv = 2;
    }
    
    return rv;
}

} // LuaPoco
