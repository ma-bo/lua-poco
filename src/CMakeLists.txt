set(LUAPOCO_SRC
    Loader.cpp
    Userdata.cpp
    StateTransfer.cpp)
    
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
    foundation/Thread.cpp
    foundation/Event.cpp
    foundation/IStream.cpp
    foundation/OStream.cpp
    foundation/PipeOStream.cpp
    foundation/PipeIStream.cpp
    )

# check if Version.h is present for 1.3.6 compatability issues.
find_path(POCO_VERSION_H Poco/Version.h)
if (POCO_VERSION_H)
    add_definitions(-DLUA_POCO_VERSION_H)
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
    add_definitions(-DPOCO_NO_AUTOMATIC_LIBS)
endif()

add_library(poco SHARED ${LUAPOCO_SRC} ${FOUNDATION_SRC})
target_link_libraries(poco ${LUA_LIB} ${LIBPocoFoundation})
set_target_properties(poco PROPERTIES PREFIX "")

install(TARGETS poco RUNTIME DESTINATION ${CMAKE_BINARY_DIR} LIBRARY DESTINATION ${CMAKE_BINARY_DIR})
