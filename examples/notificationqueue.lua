--[[ notificationqueue.lua

    This example shows how a notificationqueue can be used in two threads to 
    pass mostly arbitrary lua values between threads and via the notificationqueue.
    
    Notifications are enqueued as: "notificiation_type", ...
    The first parameter is always a string which identifies the type of notification.
    The additional parameters can be of any type sharable/copyable between threads.
    
    In this instance, two notifications are handled by the thread: "listfiles" and "checksum".
    1. The "listfiles" notification type passes a directory path as a string, 
    which will be enumerated and printed.
    2. The "checksum" notification type passes a table of file paths to be 
    checksummed and printed.
--]]

-- table of functions to handle particular notifications.
local handlers = {}
function handlers.listfiles(filepath)
    local file = require("poco.file")
    local fp = assert(file(filepath))
    print("listing: ", filepath)
    local names, err = fp:listNames()
    if names then
        for i = 1, #names do
            print(i, names[i])
        end
    else
        print("listing failed: ", filepath, err)
    end
end

function handlers.checksum(file_list)
    local checksum = require("poco.checksum")
    for i = 1, #file_list do
        print("checksum: ", file_list[i])
        local crc32 = checksum("CRC32")
        local fh = io.open(file_list[i])
        if fh then
            local data = fh:read("*a")
            if data then
                crc32:update(data)
                print(file_list[i], string.format("%x",crc32:checksum()))
            end
            fh:close()
        end
    end
    
end

-- function to be run in another thread. (separate lua_State)
function thread_entry(notification_queue, quit_event, handlers)
    print("thread_entry entered.")
    -- stop processing notifications if the event is set.
    while not quit_event:tryWait(0) do
        -- wait for up to 500ms for a notification.
        local notification_type, notification = notification_queue:waitDequeue(500)
        -- use the handlers table to handle notifications
        local handler = handlers[notification_type]
        if handler then
            handler(notification)
        elseif notification_type ~= nil then
            print("unhandled notification: ", notification_type)
        end
    end
    print("thread_entry exiting.")
end

-- require modules needed for the main thread.
local notificationqueue = require("poco.notificationqueue")
local thread = require("poco.thread")
local event = require("poco.event")

-- construct a notificationqueue, thread, and an event.
local q = notificationqueue()
local worker_thread = thread()
local quit_event = event(false)

print("starting thread.")
-- functions with no upvalues can be copied to a new lua_State.
-- poco.notificationqueue and poco.event can be shared in the new lua_State and
-- are passed to the thread's function as parameters.
-- tables are fully copied, provided all keys and values are also copyable.
worker_thread:start(thread_entry, q, quit_event, handlers)

-- send "listfiles" and "checksum" notifications.
print("enqueue listfiles : ../examples/notificationqueue")
q:enqueue("listfiles", "../examples/notificationqueue")

print("enqueue checksum : { '../examples/notificationqueue/notificationqueue.lua', '../examples/file/file.lua' }")
q:enqueue("checksum", { "../examples/notificationqueue/notificationqueue.lua", "../examples/file/file.lua" })

-- the quit_event is going to be set the main thread, but as the main thread does
-- not have anything else to do at the moment, it will wait for 1 second before
-- setting the quit event.
quit_event:tryWait(1000)
quit_event:set()

-- join the thread
print("worker_thread:join(): ", worker_thread:join())
print("exiting.")
