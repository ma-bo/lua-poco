cmake_minimum_required (VERSION 3.5)
project(luapoco)
include(ExternalProject)

### Note if this option is off, POCO libraries are expected to be found on the linker path.
option(USE_EMBEDDED_POCO "build poco library and static link into lua-poco." ON)

### Pick Lua implementation to use.
if (LUA_INCLUDE)
    ### On Windows and Linux, specify where to find the Lua header files.
    include_directories(${LUA_INCLUDE})
    ### These are mostly only useful for windows to specify:
    ### The name of the Lua DLL to link against.
    ### The path where the Lua DLL resides.
    if (LUA_LIB_PATH AND LUA_LIB_NAME)
        find_library(LUA_LIB ${LUA_LIB_NAME} PATHS ${LUA_LIB_PATH} NO_DEFAULT_PATH)
    endif()
else()
    ### For a generic unixy build that assumes the lua.h resides some where in a default CMAKE path.
    find_path(LUA_INCLUDE lua.h)
endif()

message(STATUS "lua-poco using Lua headers: ${LUA_INCLUDE}")
message(STATUS "lua-poco using Lua library: ${LUA_LIB}")

if (MSVC)
    ### lua-poco on Windows will by default use a static/interal copy of the Poco C++ library.
    ### It will also use a statically linked C/C++ runtime to avoid the redistributable mess.
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(PocoInternalSuffix "d")
    else()
        set(PocoInternalSuffix "")
    endif()
    
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()

    set(PocoInternalSuffix "mt${PocoInternalSuffix}")
elseif(MINGW)
    ## mingw-w64's build links against supplied Lua and POCO, so no suffix is present.
    ## currently mingw-w64 cannot build the embedded poco without patches.
    set(PocoInternalSuffix "")
elseif(UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if (USE_EMBEDDED_POCO)
    ### Setup internal Poco C++ library.
    #### POCO_URL can be defined at cmake invocation via a file:// URI to point at a local version of
    #### the poco-x.x.x-all.tar.gz file.  ie: file://c:/users/username/src/poco-1.11.1-all.tar.gz

    if(NOT POCO_URL)
        set(POCO_URL https://pocoproject.org/releases/poco-1.11.1/poco-1.11.1-all.tar.gz)
    endif()

    set(POCO_DIR ${CMAKE_CURRENT_BINARY_DIR}/poco_internal)
    set(POCO_INSTALL_DIR ${POCO_DIR}/install)

    include_directories(${POCO_INSTALL_DIR}/include)

    ExternalProject_Add(poco_static 
        PREFIX ${POCO_DIR}
        URL ${POCO_URL}
        CMAKE_COMMAND "${CMAKE_COMMAND}" -E env "CXXFLAGS=${CMAKE_CXX_FLAGS}" "CFLAGS=${CMAKE_C_FLAGS}" "${CMAKE_COMMAND}"
        INSTALL_DIR ${POCO_INSTALL_DIR}
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${POCO_INSTALL_DIR} -DPOCO_STATIC=ON -DPOCO_UNBUNDLED=OFF -DPOCO_MT=ON -DENABLE_MONGODB=OFF -DENABLE_PAGECOMPILER=OFF -DENABLE_PAGECOMPILER_FILE2PAGE=OFF -DENABLE_JWT=OFF -DENABLE_APACHECONNECTOR=OFF -DENABLE_DATA_MYSQL=OFF -DENABLE_DATA_POSTGRESQL=OFF -DENABLE_DATA_ODBC=OFF -DENABLE_DATA_SQLITE=OFF -DENABLE_REDIS=OFF -DENABLE_PDF=OFF -DENABLE_POCODOC=OFF -DENABLE_ACTIVERECORD=OFF -DENABLE_ACTIVERECORD_COMPILER=OFF
    )
    
    find_library(POCO_FOUNDATION_LIB PocoFoundation${PocoInternalSuffix} PATHS ${POCO_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(POCO_JSON_LIB PocoJSON${PocoInternalSuffix} PATHS ${POCO_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(POCO_ZIP_LIB PocoZip${PocoInternalSuffix} PATHS ${POCO_INSTALL_DIR} NO_DEFAULT_PATH)
else()
    find_path(POCO_HEADERS_DIR "Poco/Poco.h")
    find_library(POCO_FOUNDATION_LIB PocoFoundation${PocoInternalSuffix})
    find_library(POCO_JSON_LIB PocoJSON${PocoInternalSuffix})
    find_library(POCO_ZIP_LIB PocoZip${PocoInternalSuffix})
    include_directories(${POCO_HEADERS_DIR})
endif()



add_subdirectory(src)
