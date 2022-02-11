--[[ sharedmemory.lua
    This example demonstrates how to utilize the sharedmemory module for inter process communication.
    The sharedmemory.lua file contains the controller and the worker code.  The controller obtains and 
    transmits data, while the worker processes it when signaled by the controller.
--]]

-- load prerequisite modules.
local sharedmemory = assert(require("poco.sharedmemory"))
local namedmutex = assert(require("poco.namedmutex"))
local namedevent = assert(require("poco.namedevent"))

local data_memory_name = "sharedmemory_data_example"
local data_event_name = "sharedmemory_event"
local data_mutex_name = "sharedmemory_mutex"
local filename = "sharedmemory.lua"
local shared_memory_size = 10000
-- local lua_interpreter_path = "/usr/bin/lua"
local lua_interpreter_path = "c:/msys64/mingw64/bin/lua.exe"

-- construct synchronization mechanisms.
local data_event = assert(namedevent(data_event_name))
local data_mutex = assert(namedmutex(data_mutex_name))


-- begin worker code.
if arg[1] == "worker" then
    -- open the named sharedmemory region.
    local data_mem = assert(sharedmemory(data_memory_name, "read", shared_memory_size, false))
    -- create a memoryistream for reading from the sharedmemory region.
    local memoryistream = assert(require("poco.memoryistream"))
    local mis = assert(memoryistream(data_mem))
    local shutdown = false

    repeat
        -- set the read pointer to the beginning of the memory region.
        mis:seek("set", 0)
        -- wait for the data available event to be set, lock the mutex associated with
        -- the sharedmemory region.
        data_event:wait()
        data_mutex:lock()
        -- get the amount of data present in the shared memory encoded as text.
        -- if framing amount is 0 or the framing number is not present,
        -- interpret this condition as the sentinel value to shutdown.
        local line = mis:read("*l")
        local read_amount = tonumber(line or "")
        local data = nil
        
        if read_amount and read_amount ~= 0 then data = mis:read(read_amount)
        else shutdown = true end
        data_mutex:unlock()
        
        if data then print("worker: ", data)
        else shutdown = true end
        
    until shutdown
    print("worker: exiting!")
-- begin controller code.
else
    -- load prerequisite controller modules.
    local memoryostream = assert(require("poco.memoryostream"))
    local process = assert(require("poco.process"))
    local path = assert(require("poco.path"))
    -- construct sharedmemory and memoryostream for outputting to shared memory region.
    local data_mem = assert(sharedmemory(data_memory_name, "write", shared_memory_size, true))
    local mos = assert(memoryostream(data_mem))
   -- table containing process to launch and parameters.
    local worker_process = {
        command = lua_interpreter_path,
        workingDir = path.current(),
        args = { filename, "worker" },
    }
    -- attempt to launch the process.
    local handle = assert(process.launch(worker_process))
    io.write(string.format("controller: launched '%s %s %s' with ID: %d\n",
        worker_process.command, filename, "worker", handle:id()))

    print("controller: enter data to submit to worker, or an empty line to shutdown.")
    
    for line in io.lines() do
        -- transmit data overshared memory, then signal the worker to process it.
        local data = string.format("%d\n%s", #line, line)
        if #data <= shared_memory_size then
            data_mutex:lock()
            mos:write(data)
            mos:flush()
            data_mutex:unlock()
            data_event:set()
        else
            print("controller: entry is too big for buffer size: ", shared_memory_size)
        end
        -- reset the write position to the beginning.
        mos:seek("set", 0)
        -- if an empty line was sent, shutdown.
        if #line == 0 then break end
    end
    
    -- check for a clean exit, or print out the error result and the error message.
    local result = handle:wait()
    if result == 0 then print("controller: worker completed successfully.")
    else print("controller: worker failed with status: ", result) end
end
