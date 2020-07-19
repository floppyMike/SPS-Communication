OPTION(NO_LIB "Disable libnodave" OFF)

add_executable(SPSCom main.cpp)

target_include_directories(SPSCom PRIVATE ../include)
target_include_directories(SPSCom PRIVATE ../extern/libnodave/include)
target_include_directories(SPSCom PRIVATE ../extern/asio/include)
target_include_directories(SPSCom PRIVATE ../extern/rapidjson/include)

if(NO_LIB)
	add_compile_definitions(SPS_NOT_AVAILABLE)
endif(NO_LIB)

if(UNIX AND NOT APPLE)
	if(NOT NO_LIB)
		target_link_libraries(SPSCom nodave)
	endif(NOT NO_LIB)
	target_link_libraries(SPSCom pthread)
elseif(WIN32)
	target_link_directories(SPSCom PRIVATE ../win32)
	if(NOT NO_LIB)
		target_link_libraries(SPSCom libnodave)
	endif(NOT NO_LIB)
endif()