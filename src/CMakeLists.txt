set(FOUNDATION_SRC
	Userdata.cpp
	File.cpp)

add_library(foundation shared ${FOUNDATION_SRC})
target_link_libraries(foundation ${LIBLUA51} ${LIBPocoFoundation})
