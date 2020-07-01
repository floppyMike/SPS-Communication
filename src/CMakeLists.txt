add_executable(SPSCom main.cpp)

target_include_directories(SPSCom PRIVATE ../include)
target_include_directories(SPSCom PRIVATE ../extern/libnodave/include)
target_include_directories(SPSCom PRIVATE ../extern/asio/include)
target_include_directories(SPSCom PRIVATE ../extern/rapidjson/include)

if(UNIX AND NOT APPLE)
	target_link_libraries(SPSCom nodave)
	target_link_libraries(SPSCom pthread)
elseif(WIN32)
	target_link_directories(SPSCom PRIVATE win32)
	target_link_libraries(SPSCom libnodave)
endif()