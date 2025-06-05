--[[ timestamp.lua
    This example shows how the varius capabilities of the timestamp module.
    Timestamps are represented in UTC time values.

    Any time timestamp value is returned to Lua, they are returned as 64-bit integers encoded in
    strings.  The rationale for this is due to lua_Number size limitations.
--]]

-- load the poco.timestamp module.
local timestamp = require("poco.timestamp")

-- capture the current timestamp.
local ts1 = timestamp()
-- print out the microseconds since UTC epoch.
print(string.format("\nmicroseconds since the UTC epoch (%s)", os.date("!%c", 0)))
print("\t", ts1:epochMicroseconds(), "\n")

-- The epochTime function's return value is equivalent to os.time()'s return value
-- as the microsecond resolution is truncated to seconds.
print("\nlocal date and time: (UTC)")
print("\t", os.date("!%c", ts1:epochTime()), "\n")
print("local date and time:")
print("\t", os.date("%c", ts1:epochTime()), "\n")

-- count how many loop iterations pass before a change is noticed.
-- note: on Windows, the elapsed value will be a bit higher due to minimum OS scheduling latencies.
-- on Linux and else where, there should only be a single loop pass with some
-- small number of microseconds elapsed.

-- Note that values used for adding/subtracting from a timestamp, and the values themselves inside
-- of Lua are 64-bit integers, but in string representation.  The rationale for the string representation
-- is that luajit, Lua 5.1 and 5.2 do not have a 64-bit capable number type.
local elapsed = "0"
local counter = 0
repeat
    elapsed = ts1:elapsed()
    counter = counter + 1
until elapsed ~= "0"

print("\nmicroseconds taken to notice a timestamp delta:")
print("\t", elapsed, "\n")
print("loop iterations to notice a timestamp delta:")
print("\t", counter, "\n")

local ts2 = timestamp()

-- timestamp userdata can tested for equality or compared.
print("\ntimestamp 1 comparisons with timestamp 2:")
print("ts1 == ts2:\t", ts1 == ts2)
print("ts1 < ts2:\t", ts1 < ts2)
print("ts2 < ts1:\t", ts2 < ts1)

-- timestamp userdata can be shifted  with addition or subtraction of microseconds.
-- the value passed is an 64-bit integer in the form of a string.
print("\ntimestamp shifting via addition and subtraction:")
print("ts2 + 20000:\t", (ts2 + tostring(20000)):epochMicroseconds())
print("ts2 - 10000:\t", (ts2 - tostring(10000)):epochMicroseconds())

-- one timestamp can be subtracted from another in order to create a delta.
print("\ntimestamp deltas:")
print("ts2 - ts1:\t", (ts2 - ts1):epochMicroseconds())

-- even more granular timestamp's are possible by calling utcTime()
print("\n100s of nanosecond intervals since midnight, October 15, 1582:")
print("timestamp():utcTime():\t", timestamp():utcTime())
