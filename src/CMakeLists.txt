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
    foundation/FileIStream.cpp
    foundation/FileOStream.cpp
    foundation/InflatingIStream.cpp
    foundation/InflatingOStream.cpp
    foundation/DeflatingIStream.cpp
    foundation/DeflatingOStream.cpp
    foundation/Path.cpp
    foundation/Notification.cpp
    foundation/NotificationFactory.cpp
    foundation/NotificationQueue.cpp
    foundation/Buffer.cpp
    foundation/MemoryIStream.cpp
    foundation/MemoryOStream.cpp
    foundation/TeeOStream.cpp
    foundation/TeeIStream.cpp
    foundation/TaskManager.cpp
    foundation/JSON.cpp
    foundation/Compress.cpp
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    add_definitions(-DPOCO_NO_AUTOMATIC_LIBS)
    if(MSVC)
        find_library(LIBIPHLPAPI iphlpapi)
        set(PLATFORM_EXTRAS ${LIBIPHLPAPI})
    endif()
elseif(UNIX)
    find_library(LIBPTHREAD pthread)
    find_library(LIBRT rt)
    set(PLATFORM_EXTRAS ${LIBPTHREAD} ${LIBRT})
endif()

set(CXX_STANDARD_REQUIRED ON)

if(USE_EMBEDDED_POCO)
    link_directories(${POCO_INSTALL_DIR}/lib)
endif()

add_library(poco SHARED ${LUAPOCO_SRC} ${FOUNDATION_SRC})

if(USE_EMBEDDED_POCO)
    add_dependencies(poco poco_static)
endif()

target_link_libraries(poco ${LUA_LIB} PocoFoundation${PocoInternalSuffix} PocoJSON${PocoInternalSuffix} PocoZip${PocoInternalSuffix} ${PLATFORM_EXTRAS})
set_target_properties(poco PROPERTIES PREFIX "" CXX_STANDARD 14)

install(TARGETS poco RUNTIME DESTINATION ${CMAKE_BINARY_DIR} LIBRARY DESTINATION ${CMAKE_BINARY_DIR})
