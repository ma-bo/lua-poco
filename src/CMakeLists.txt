set(LUAPOCO_SRC
	Loader.cpp
	Userdata.cpp)
	
set(FOUNDATION_SRC
	foundation/File.cpp
	foundation/Timestamp.cpp
	foundation/DynamicAny.cpp
	foundation/RegularExpression.cpp
	foundation/Checksum.cpp
	foundation/Environment.cpp
	foundation/Pipe.cpp
	foundation/NamedEvent.cpp
	foundation/NamedMutex.cpp
	foundation/ProcessHandle.cpp
	foundation/Process.cpp
	foundation/Semaphore.cpp
	foundation/FastMutex.cpp
	foundation/Mutex.cpp
	)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(poco SHARED ${LUAPOCO_SRC} ${FOUNDATION_SRC})
target_link_libraries(poco ${LUA_LIB} ${LIBPocoFoundation})
set_target_properties(poco PROPERTIES PREFIX "")

install(TARGETS poco LIBRARY DESTINATION ${CMAKE_BINARY_DIR})
