--[[ taskmanager.lua
    This example shows how to run Lua code in a separate Lua states with a TaskManager thread pool.
--]]

local function environment_task(manager, task, output_file)
    local env = assert(require("poco.environment"))
    local of = assert(io.open(output_file, "w"))
    for k,v in pairs(task) do of:write(tostring(k), "\t", tostring(v), "\n") end
    for k,v in pairs(getmetatable(task)) do of:write(tostring(k), "\t", tostring(v), "\n") end
    of:write("osName: ", env.osName(), "\n")
    of:write("osArchitecture: ", env.osArchitecture(), "\n")
    of:write("osDisplayName: ", env.osDisplayName(), "\n")
    of:write("osVersion: ", env.osVersion(), "\n")
    of:write("libraryVersion:", env.libraryVersion(), "\n")
    of:close()
    return
end

local function observer_task(manager, task)

    local function print_notification(n)
        print("task:", n.task)
        print("name:", n.name)
        print("type:", n.type)
        print("code:", n.code)
        print("message:", n.message)
        print("progress:", n.progress)
    end

    local n = {}
    
    while true do
        local environment_task_ud = nil
        
        if manager:dequeueNotification(n, 100) then
            print_notification(n)
            
            if n.name == "environment_task" and n.type ~= "started" then
                print("environment task done")
                break
            end
        end
    end
    print("observer_task returning")
end

-- Main code that spawns the thread to do some work, then waits for it to complete.
-- Add a local namespace for poco modules.
local poco = {}
poco.path = require("poco.path")
poco.taskmanager = require("poco.taskmanager")

-- Construct a new taskmanager
local tm = assert(poco.taskmanager())

-- Start the thread and pass a path as a parameter for it to write to.
local output_file = poco.path.temp() .. "wtdata.txt"
print(string.format("starting environment_task outputting to %s", output_file))

local obs = assert(tm:start("observer_task", observer_task))
local eth = assert(tm:start("environment_task", environment_task, output_file))

tm:joinAll()
print("done.")
