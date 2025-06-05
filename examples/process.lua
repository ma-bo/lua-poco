--[[ process.lua
    This example shows how to spawn an external process, send and receive input to it via pipes.
    In this example, the environment.lua example will be run in a separate process.
--]]

local lua_cmd = os.getenv("LUACMD") or "/bin/lua"

-- require prerequisites.
local process = require("poco.process")
local pipe = require("poco.pipe")
local path = require("poco.path")
local pipeistream = require("poco.pipeistream")

-- construct anonymous pipes to attach to the process.
local read_pipe = assert(pipe())
local err_pipe = assert(pipe())

-- Table containing process to launch, and all the optional parameters.
local filename = "environment.lua"
local worker_config = {
    command = lua_cmd,
    workingDir = path.current(),
    args = { filename },
    outPipe = read_pipe,
    errPipe = err_pipe
}

-- attempt to launch the process
local handle, error_msg = assert(process.launch(worker_config))
io.write(string.format("%s spawned with ID: %d\n", worker_config.command, handle:id()))

local pipeis = assert(pipeistream(read_pipe))
local output = pipeis:read("*a")
if output then io.write(output) end

-- clean up after spawned process and retrieve the result with wait().
local result = handle:wait()
-- check for a clean exit, or print out the error result and the error message.
if result == 0 then
    io.write(string.format("\n%s completed successfully.\n", worker_config.command))
else
    local amount, msg = err_pipe:readBytes()
    io.write(string.format("\n%s failed status: %d msg: %s\n", worker_config.command, result, msg))
end
