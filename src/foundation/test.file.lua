local foundation = assert(require("foundation"))
local file = assert(foundation.File("test1"))

print("file: ", "test1")
print("exists", file:exists())
print("size: ", file:getSize())
