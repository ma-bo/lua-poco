--[[ notificationqueue.lua

    This example shows how a notificationqueue can be used in two threads to 
    pass mostly arbitrary lua values between threads and via the notificationqueue.
    
    Notifications are enqueued as: "notificiation_type", ...
    The first parameter is always a string which identifies the type of notification.
    The additional parameters can be of any type sharable/copyable between threads.
--]]

-- require modules needed for the main thread.
local notificationqueue = require("poco.notificationqueue")
local thread = require("poco.thread")

-- function to be run in another thread and separate lua_State.
function worker(notification_queue)
    -- wait for up to 500ms for a notification.
    repeat
        local notification_type, notification = notification_queue:waitDequeue(500)
        print("notification received:", notification_type, type(notification))
        
        if notification_type == "quit" then
            quit = true
        else
            -- do work on notifications.
        end
    until quit
end

-- create worker thread, and a notification queue.
local wt = assert(thread())
local q = assert(notificationqueue())

-- functions with no upvalues can be copied to a new lua_State.
-- poco.notificationqueue  can be shared in the new lua_State and
-- is passed to the thread's function a parameter.
-- tables are fully copied, provided all keys and values are also copyable.
print("posting notifications to thread.")
assert(q:enqueue("hello world"))
assert(q:enqueue("table", { "hello", "world" }))

print("queue size: ", q:size())
print("hasIdleThreads: ", q:hasIdleThreads())

assert(wt:start(worker, q))
assert(q:enqueue("quit"))

-- join the thread
assert(wt:join())
print("thread complete, result: ", wt:result())
