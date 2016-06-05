/// Functions for working with processes.
// @module process

#include "Loader.h"
#include "Process.h"
#include "ProcessHandle.h"
#include "Pipe.h"
#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/Version.h"

int luaopen_poco_process(lua_State* L)
{
    struct LuaPoco::UserdataMethod methods[] = 
    {
        { "kill", LuaPoco::Process::kill },
        { "id", LuaPoco::Process::id },
        { "requestTermination", LuaPoco::Process::requestTermination },
        { "times", LuaPoco::Process::times },
        { "launch", LuaPoco::Process::launch },
        { NULL, NULL}
    };
    
    if (LuaPoco::loadMetatables(L))
    {
        lua_createtable(L, 0, 5);
        setMetatableFunctions(L, methods);
    }
    
    return 1;
}

namespace LuaPoco
{

/// Kills the process with the given pid.
// @int processId
// @return true or nil. (error)
// @function kill
int Process::kill(lua_State* L)
{
    int rv = 0;
    try
    {
        Poco::Process::PID pid = luaL_checknumber(L, 1);
        Poco::Process::kill(pid);
        lua_pushboolean(L, 1);
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

/// Gets the pid for the current process.
// @return pid as a Lua number.
// @function id
int Process::id(lua_State* L)
{
    int rv = 0;
    try
    {
        lua_Number id = Poco::Process::id();
        lua_pushnumber(L, id);
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

/// Requests termination of the process with the give process id. 
// On Unix platforms, this will send a SIGINT to the process and thus work with arbitrary processes.
// On other platforms, a global event flag will be set. Setting the flag will cause serverapp:waitForTerminationRequest() to return.
// Therefore this will only work with applications based on poco.serverapp
// @int pid process id to request to terminate.
// @return true or nil. (error)
// @function requestTermination
int Process::requestTermination(lua_State* L)
{
    int rv = 0;
    try
    {
        Poco::Process::PID pid = luaL_checknumber(L, 1);
        Poco::Process::requestTermination(pid);
        lua_pushboolean(L, 1);
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

/// Returns the number of seconds spent by the current process in user and kernel mode.
// @return usertime as a Lua number.
// @return kernel as a Lua number.
// @function times
int Process::times(lua_State* L)
{
    int rv = 0;
    try
    {
        long user;
        long kernel;
        
        Poco::Process::times(user, kernel);
        lua_pushinteger(L, user);
        lua_pushinteger(L, kernel);
        rv = 2;
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

namespace
{

const char* getCommand(lua_State* L)
{
    const char* command = NULL;
    
    lua_getfield(L, -1, "command");
    if (lua_isstring(L, -1))
        command = lua_tostring(L, -1);
    
    lua_pop(L, 1);
    
    return command;
}

const char* getWorkingDir(lua_State* L)
{
    const char* wd = NULL;
    
    lua_getfield(L, -1, "workingDir");
    if (lua_isstring(L, -1))
        wd = lua_tostring(L, -1);
    
    lua_pop(L, 1);
    
    return wd;
}

void getPipes(lua_State* L, PipeUserdata*& inPipe, PipeUserdata*& outPipe, PipeUserdata*& errPipe)
{
    lua_getfield(L, -1, "inPipe");
    if (lua_isuserdata(L, -1)) inPipe = dynamic_cast<PipeUserdata*>(getPrivateUserdata(L, -1));
    lua_pop(L, 1);
    
    lua_getfield(L, -1, "outPipe");
    if (lua_isuserdata(L, -1)) outPipe = dynamic_cast<PipeUserdata*>(getPrivateUserdata(L, -1));
    lua_pop(L, 1);
    
    lua_getfield(L, -1, "errPipe");
    if (lua_isuserdata(L, -1)) errPipe = dynamic_cast<PipeUserdata*>(getPrivateUserdata(L, -1));
    lua_pop(L, 1);
}

#if POCO_VERSION >= 0x01040400
// requires that the env table is at -1 on the stack
bool getEnv(lua_State* L, Poco::Process::Env& env)
{
    bool result = false;
    
    int cleanTop = lua_gettop(L);
    lua_getfield(L, -1, "env");
    
    if (lua_istable(L, -1))
    {
        result = true;
        
        lua_pushnil(L);
        while (lua_next(L, -2))
        {
            // key and value must be strings
            if (!lua_isstring(L, -1) || lua_isstring(L, -2))
                break;
            
            const char* value = lua_tostring(L, -1);
            const char* key = lua_tostring(L, -2);
            env[key] = value;
            // pop the value, leaving the key at -1 for next()
            lua_pop(L, 1);
        }
    }
    lua_pop(L, lua_gettop(L) - cleanTop);
    
    return result;
}
#endif

void getArgs(lua_State* L, Poco::Process::Args& args)
{
    size_t i = 1;
    
    lua_getfield(L, -1, "args");
    if (lua_istable(L, -1))
    {
        // start at index 1
        for (lua_rawgeti(L, -1, i); lua_isstring(L, -1); lua_rawgeti(L, -1, i))
        {
            const char* value = lua_tostring(L, -1);
            args.push_back(value);
            // pop value just obtained
            lua_pop(L, 1);
            ++i;
        }
        // pop nil result from 1 past the last valid index
        lua_pop(L, 1);
    }
    // pop args table
    lua_pop(L, 1);
}

}

///
// @field command the program to launch.
// @field workingDir [optional] a path to be used as the working directory for the command.
// On Poco Library versions older than 1.4.4, the supplied environment field will be ignored.
// @field inPipe [optional] a poco.pipe userdata that will be attached to the commands stdin.
// @field outPipe [optional] a poco.pipe userdata that will be attached to the commands stdout.
// @field errPipe [optional] a poco.pipe userdata that will be attached to the commands stderr.
// @field env [optional] a table representing key/value pairs for the commands process environment.
// On Poco Library versions older than 1.4.4, the supplied environment field will be ignored.
// @table launchParam

/// Launches a new process with command and returns a processhandle userdata.
// The new program is launched directly; the command shell is not invoked.
// @param lauchParam table of parameters for launch.
// @return processhandle userdata or nil. (error)
// @return error message.
// @function launch
// @see launchParam
int Process::launch(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    
    // required parameters
    const char* command = getCommand(L);
    if (!command)
    {
        lua_pushnil(L);
        lua_pushstring(L, "must at least have a command in launch table");
        return 2;
    }
    
    // optional table parameters
    const char* workingDir = getWorkingDir(L);
    
    Poco::Process::Args args;
    getArgs(L, args);
    
    PipeUserdata* inPipeUd = NULL;
    PipeUserdata* outPipeUd = NULL;
    PipeUserdata* errPipeUd = NULL;
    getPipes(L, inPipeUd, outPipeUd, errPipeUd);
    
    Poco::Pipe* inPipe = inPipeUd ? &inPipeUd->mPipe : NULL;
    Poco::Pipe* outPipe = outPipeUd ? &outPipeUd->mPipe : NULL;
    Poco::Pipe* errPipe = errPipeUd ? &errPipeUd->mPipe : NULL;

    bool haveEnv = false;
#if POCO_VERSION >= 0x01040400
    Poco::Process::Env env;
    haveEnv = getEnv(L, env);
#endif
    
    
    try
    {
        if (haveEnv)
        {
#if POCO_VERSION >= 0x01040400
            Poco::ProcessHandle ph = Poco::Process::launch(command, args, 
                workingDir ? workingDir : Poco::Path::current(),
                inPipe, outPipe, errPipe, env);
            ProcessHandleUserdata* phud = new(lua_newuserdata(L, sizeof *phud)) ProcessHandleUserdata(ph);
            setupPocoUserdata(L, phud, POCO_PROCESSHANDLE_METATABLE_NAME);
            rv = 1;
#endif
        }
        else
        {
            Poco::ProcessHandle ph = Poco::Process::launch(command, args, 
#if POCO_VERSION >= 0x01040400
                workingDir ? workingDir : Poco::Path::current(),
#endif
                inPipe, outPipe, errPipe);

            ProcessHandleUserdata* phud = new(lua_newuserdata(L, sizeof *phud)) ProcessHandleUserdata(ph);
            setupPocoUserdata(L, phud, POCO_PROCESSHANDLE_METATABLE_NAME);
            rv = 1;
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

} // LuaPoco
