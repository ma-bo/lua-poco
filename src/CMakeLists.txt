# Build core components for sub libraries

set(LUAPOCO_SRC
	Userdata.cpp)

add_library(luapoco-core SHARED ${LUAPOCO_SRC})
target_link_libraries(luapoco-core ${LIBLUA51})

# Build foundation binding
add_subdirectory(foundation)
