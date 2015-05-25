local path = require("poco.path")

-- freestanding path related functions
print("\n-- Platform information:")
print("Path Seperator:              ", path.pathSeparator())
print("Directory Seperator:         ", path.separator())
print("Temporary Directory:         ", path.temp())
print("Null Device:                 ", path.nullDevice())
print("Home Directory:              ", path.home())
print("Current Working Directory:   ", path.current())
print("Filesystem Roots:")
local roots = path.listRoots()
for i = 1, #roots do
    print(i, roots[i])
end

-- Windows style path.
local windows_path_string = "C:\\Windows\\System32\\kernel32.dll"
print("\n-- Sample Windows Path:    ", windows_path_string)
-- create a Windows style path userdata and print information about it.
local wp = assert(path(windows_path_string, "WINDOWS", true))
print("depth:                     ", wp:depth())
print("isAbsolute:                ", wp:isAbsolute())
print("isRelative:                ", wp:isRelative())
print("isDirectory:               ", wp:isDirectory())
print("isFile:                    ", wp:isFile())
print("Device:                    ", wp:getDevice())
print("FileName:                  ", wp:getFileName())

-- Unix style path.
local unix_path_string = "/usr/include/lua5.1/"
-- create a Unix style path userdata and print information about it.
print("\n-- Sample Unix Path:         ", unix_path_string)
local up = assert(path(unix_path_string, "UNIX", true))
print("depth:                     ", up:depth())
print("isAbsolute:                ", up:isAbsolute())
print("isRelative:                ", up:isRelative())
print("isDirectory:               ", up:isDirectory())
print("isFile:                    ", up:isFile())
print("FileName:                  ", up:getFileName())

-- perform some operations/manipulations to the unix style path and print.
print("\n-- Path Operations:      ", up:toString())

-- create a new relative unix path and append it to the first unix path.
local up2 = assert(path("extra/directory/", "UNIX"))
up:append(up2)
print("append relative:             ", up2:toString())

-- print out each directory in the appended path.
print("iterate directories:         ", up:toString())
for i = 1, up:depth() do
    print(i, up:directory(i))
end

-- pop the two appended directories.
up:popDirectory()
print("popDirectory:                ", up:toString())
up:popDirectory()
print("popDirectory:                ", up:toString())
