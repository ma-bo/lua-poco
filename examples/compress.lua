--[[ compress.lua
    This example shows how a the compress module can be used to create zip files and write them to
    a compatible ostream.  Files and directories can be added via istreams and filesystem paths.
--]]

-- the ostream could be other types, such as pipeostream, teeostream, memoryostream, etc.
local fostream = require("poco.fileostream")
local compress = require("poco.zip.compress")
local path = require("poco.path")

-- write the zip file to the output stream in the temp directory.
local output_zip_path = path(path.temp())
output_zip_path:setFileName("compress_example.zip")
local zfs = assert(fostream(output_zip_path:toString()))

-- create compress instance with the fileostream.
--- compress stores a reference to the fileostream to prevent GC, 
--- so a reference does not need to be maintained by the caller.
--- additionally, the default true parameter for seekable must be false for 
--- non-seekable istreams such as pipeistream, socketistream, etc.
local c = assert(compress(zfs))

-- print the list of file extensions that will get stored in the zipfile
-- via the STORE method rather than DEFLATE.
print("\nFile extensions using STORE method: ")
for k,v in pairs(c:getStoreExtensions()) do print(k,v) end

-- store a comment for the zip file and print it out.
c:setZipComment("example comment")
print("\nZip file comment:", c:getZipComment())

-- assume current working directory is the directory containing the examples.
local examples_path = assert(path(path.current()))
-- add all files and directories under the 'examples' directory to the zip file.
io.write(string.format("\nAdding recursive directory: %s\nto: %s\n", examples_path:toString(), output_zip_path:toString()))
assert(c:addRecursive(examples_path, "examples"))

-- add README.md to readme/README.md via the addFile function.
examples_path:popDirectory()
examples_path:setFileName("README.md")
io.write(string.format("\nAdding File: %s\nto: %s\n", examples_path:toString(), output_zip_path:toString()))
assert(c:addFile(examples_path, "readme/README.md"))

--[[ alternatively one could add a file via an poco istream via the addIStream function.
-- note that this source could be a memoryistream, pipeistream, socketistream, etc.
local fistream = assert(require("poco.fileistream"))
local fis = assert(fistream(examples_path:toString()))
--]]

-- finalize the zip file.
c:close()
