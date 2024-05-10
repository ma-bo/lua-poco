--[[ thread.lua
    This example shows how to run Lua code in a separate Lua state via a native thread.
--]]

-- worker_thread()
-- The thread entrypoint function.  The name is arbitrary.

-- Note: lexically captured variables (upvalues) of the thread entrypoint function
-- are not preserved in the new Lua state which hosts the thread, only the values
-- passed as arguments.
local function worker_thread(output_file)
    local env = assert(require("poco.environment"))
    local of = assert(io.open(output_file, "w"))
    of:write("osName: ", env.osName(), "\n")
    of:write("osArchitecture: ", env.osArchitecture(), "\n")
    of:write("osDisplayName: ", env.osDisplayName(), "\n")
    of:write("osVersion: ", env.osVersion(), "\n")
    of:write("libraryVersion:", env.libraryVersion(), "\n")
    of:close()
    return
end

-- Main code that spawns the thread to do some work, then waits for it to complete.
-- Add a local namespace for poco modules.
local poco = {}
poco.path = assert(require("poco.path"))
poco.thread = assert(require("poco.thread"))

-- Construct a new thread
local wt = assert(poco.thread())

-- Start the thread and pass a path as a parameter for it to write to.
local worker_output_file = poco.path.temp() .. "wtdata.txt"
print(string.format("starting worker_thread %s outputting to %s", wt, worker_output_file))
assert(wt:start(worker_thread, worker_output_file))

print("waiting for thread to finish.")
assert(wt:join())

print("thread finished.")
