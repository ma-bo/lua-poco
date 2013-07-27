local foundation = assert(require("poco"))
local ff = assert(foundation.File("test1"))

print("file: ", ff.path)
print("exists: ", ff.exists)
print("size: ", ff:size)
