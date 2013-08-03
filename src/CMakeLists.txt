set(LUAPOCO_SRC
	Loader.cpp
	Userdata.cpp)
	
set(FOUNDATION_SRC
	foundation/File.cpp
	foundation/Timestamp.cpp
	foundation/Int64.cpp
	foundation/RegularExpression.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(poco SHARED ${LUAPOCO_SRC} ${FOUNDATION_SRC})
target_link_libraries(poco ${LIBLUA51} ${LIBPocoFoundation})
set_target_properties(poco PROPERTIES PREFIX "")

install(TARGETS poco LIBRARY DESTINATION ${CMAKE_BINARY_DIR})
