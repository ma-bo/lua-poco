--[[ temporaryfile.lua
    This example demonstrates the temporaryfile module.  It generates a temporary file path, that
    is automatically removed upon destruction of the userdata.

    The temporaryfile userdata inherits all methods from the file module.
--]]

temporaryfile = require("poco.temporaryfile")
-- generate a temporary file.
local tf1 = assert(temporaryfile())
print("path: ", tf1:path())
print("exists: ", tf1:exists())

-- create the file on the filesystem, and set its size to 100.
print("createFile: ", tf1:createFile())
print("exists: ", tf1:exists())
print("setSize: ", tf1:setSize(100))
print("size: ", tf1:size())

-- generate a temporary name:
print("temporary name: ", temporaryfile.tempName())
