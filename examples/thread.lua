--[[ thread.lua
    This example shows how threads and mutexes can be used to calculate average
    ping times for multiple hostnames concurrently, and write the output to a file.
--]]

--- ping_average(), the thread entrypoint function.
-- pings the specified host 10 times, and computes the average
-- time and writes the result to the specified file name.

-- Note: lexically captured variables (upvalues) of the thread entrypoint function
-- are not preserved in the new Lua state which hosts the thread, only the values
-- passed as arguments.
local function ping_average(host, file_mutex, filename)
    -- Windows command: ping -t 10 hostname
    -- Posix command:   ping -c 10 hostname
    local environment = require("poco.environment")
    local ping_command = string.format("ping %s 10 %s", environment.osName():find("Windows") and "-n" or "-c", host)

    local ping_output = io.popen(ping_command)
    if ping_output then
        local count = 0
        local sum = 0

        -- collect ping times from ping output.
        for line in ping_output:lines() do
            ping_value = line:match(" time=([%d%.]+) ?ms")
            if ping_value then
                sum = sum + tonumber(ping_value)
                count = count + 1
            end
        end

        ping_output:close()

        -- use mutex to serialize threaded access to the results file.
        -- calculate result for the host, and write to the file.
        if count > 0 then
            file_mutex:lock()
            local file = io.open(filename, "a")
            if file then
                file:write(string.format("host: %s - average (%d pings): %f\n", host, count, sum / count))
                file:close()
            end
            file_mutex:unlock()
        end
    end
end

-- add a local namespace for poco modules.
local poco = {}
poco.thread = require("poco.thread")
poco.fastmutex = require("poco.fastmutex")

-- table to hold a thread and the associated hostname that it will ping.
-- { host = "host", thread = poco.thread() }
local ping_workers = {}
local results_mutex = poco.fastmutex()
local results_file = "ping_results.txt"

print("enter hostnames to ping, one per line, then EOF to finish.")
print("output written to: ", results_file)
-- read one hostname per line.
-- start one thread per hostname.
for line in io.lines() do
    local worker = { host = line, thread = poco.thread() }
    table.insert(ping_workers, worker)
    assert(worker.thread:start(ping_average, worker.host, results_mutex, results_file))
end

io.write("waiting for ", #ping_workers, " ping worker threads to finish.\n")
for i = 1, #ping_workers do
    print("joining thread for host: ", ping_workers[i].thread:id(), ping_workers[i].host)
    ping_workers[i].thread:join()
end
print("done.")
