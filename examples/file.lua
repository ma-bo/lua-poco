-- load the poco.file module
local file = require("poco.file")

local example_file_name = "example.file.txt"
print("\nexample_file_name: ", example_file_name)
-- if a module's table has a new() function, you can also use a function call on
-- the that table to call the new() function.
local example_file = assert(file(example_file_name))
print("\texists:     ", example_file:exists())
print("\tcreateFile: ", example_file:createFile())
print("\texists:     ", example_file:exists())
print("\tsize:       ", example_file:size())
print("\tsetSize(99):", example_file:setSize(99))
print("\tsize:       ", example_file:size())
print("\tcanRead:    ", example_file:canRead())
print("\tcanWrite:   ", example_file:canWrite())
print("\tcanExecute: ", example_file:canExecute())
print("\tisFile:     ", example_file:isFile())
print("\tisDevice:   ", example_file:isDevice())
print("\tisDirectory:", example_file:isDirectory())
print("\tisHidden:   ", example_file:isHidden())
print("\tisLink:     ", example_file:isLink())
-- created() returns a poco.timestamp userdata, which we will use epochTime() to
-- get a time value compatible with os.date().
print("\tcreated:   ", os.date("%c", example_file:created():epochTime()))
print("\tremove:    ", example_file:remove())
print("\texists:    ", example_file:exists())

print("\nexample_directory: .")
local example_dir = assert(file("."))
print("\tpath:       ", example_dir:path())
print("\texists:     ", example_dir:exists())
print("\tisFile:     ", example_dir:isFile())
print("\tisDirectory:", example_dir:isDirectory())
local files = example_dir:listNames()
print("\tlistFiles:  ", files)
print("\tentries:    ", #files)
for i=1, #files do print("\t" .. files[i]) end
