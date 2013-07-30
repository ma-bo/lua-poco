local poco = assert(require("poco"))
local ff = assert(poco.File("test1"))

local fileType
if ff:isFile() then
	fileType = "file"
elseif ff:isDirectory() then
	fileType = "directory"
elseif ff:isDevice() then
	fileType = "device"
else
	fileType = "none"
end

print("file: ", ff:path())
print("exists: ", ff:exists())
print("type: ", fileType)
print("size: ", ff:size())
