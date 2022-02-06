--[[ process.lua
    This example shows how to spawn an external process, send and receive input to it via pipes.
    In this example, gpg will be used to encrypt this lua file with a passphrase supplied via stdin.
    The encrypted data will be read from stdout and printed to this processes stdout.

    In the case of an error, a non zero result will be returned and error message received on the
    stderr pipe will be printed.
--]]

-- require prerequisites.
local process = assert(require("poco.process"))
local pipe = assert(require("poco.pipe"))
local path = assert(require("poco.path"))

-- construct 3 anonymous pipes to attach to the process.
local write_pipe = assert(pipe())
local read_pipe = assert(pipe())
local err_pipe = assert(pipe())

-- Table containing process to launch, and all the optional parameters.
local filename = "process.lua"
local worker_proc = {
    -- command = "/usr/bin/gpg"
    command = "C:/msys64/usr/bin/gpg.exe",
    workingDir = path.current(),
    args = { "--batch", "--passphrase-fd", "0", "-c", "-a", "-o", "-", filename },
    inPipe = write_pipe,
    outPipe = read_pipe,
    errPipe = err_pipe
}

-- attempt to launch the process
local handle, error_msg = assert(process.launch(worker_proc))
io.write(string.format("%s spawned with ID: %d\n", worker_proc.command, handle:id()))

-- send the passphrase to GPG via stdin and close the input stream pipe.
write_pipe:writeBytes("Super!Secret!Passphrase!")
write_pipe:close()

-- read all the response data until the pipe is closed.
while true do
    local amount, msg = read_pipe:readBytes()

    if amount == nil then
        print("pipe error: ", msg)
        break
    elseif amount == 0 then
        -- pipe closed.
        break
    else
        io.write(msg)
        io.flush()
    end
end

-- clean up after spawned process and retrieve the result with wait().
local result = handle:wait()
-- check for a clean exit, or print out the error result and the error message.
if result == 0 then
    io.write(string.format("\n%s completed successfully.\n", worker_proc.command))
else
    local amount, msg = err_pipe:readBytes()
    io.write(string.format("\n%s failed status: %d msg: %s\n", worker_proc.command, result, msg))
end

