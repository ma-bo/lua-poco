--[[ condition.lua
    This example shows how to use the condition module to signal threads.  This module differs from
    the event module in that conditions accept a mutex as an argument.

    In the example, the main thread reads stdin to trigger the condition.  The threads do some work
    with a shared resource under the protection of a mutex.  The mutex is unlocked when
    condition:wait() is called, and locked when condition:wait() returns.
--]]

-- this function will be copied to its own lua state, and thread_main will be run from a thread.
-- no upvalues are copied to the new lua state, but function parameters are copied from main.
local function thread_main(id, condition, mutex, data_file, quit_event)
    print("thread starting: ", id)
    while true do
        condition:wait(mutex)
        if quit_event:tryWait(0) then break end
        -- do lots of important work on the shared resource below!
        local df = assert(io.open(data_file:path(), "a"))
        df:write(string.format("%d: %s %f\n", id, os.date(), os.clock()), "\n")
        df:close()
    end
    -- release mutex on quit.
    print("thread exiting: ", id)
    mutex:unlock()
end

-- prerequisite modules
local thread = require("poco.thread")
local fmutex = require("poco.fastmutex")
local condition = require("poco.condition")
local event = require("poco.event")
local temporaryfile = require("poco.temporaryfile")

-- module instances
local worker_thread1 = assert(thread())
local worker_thread2 = assert(thread())
local mtx = assert(fmutex())
local dowork_cond = assert(condition())
local data_file = assert(temporaryfile())
-- use a non-auto resetting event to enable all threads to quit when signaled.
local quit_event = assert(event(false))

-- lock access to the shared resource.  the mutex will be unlocked when calling condition:wait()
-- and locked again when condition:wait() returns.
mtx:lock()

-- start worker threads.
print("threads will output data to: ", data_file:path())
assert(worker_thread1:start(thread_main, worker_thread1:id(), dowork_cond, mtx, data_file, quit_event))
assert(worker_thread2:start(thread_main, worker_thread2:id(), dowork_cond, mtx, data_file, quit_event))

-- main loop to trigger conditions.
print("main: enter any data to trigger threads to do work, or enter an empty line to quit.")
repeat
    line = io.read()
    if line == "" then quit_event:set() end
    dowork_cond:broadcast()
    print("main: broadcasting condition.")
until quit_event:tryWait(0)

print("main: stopping.")
worker_thread1:join()
worker_thread2:join()
print("main: threads completed.")

-- print out data from the temporary file before it gets removed.
mtx:lock()
for line in io.lines(data_file:path()) do
    print(line)
end
mtx:unlock()
