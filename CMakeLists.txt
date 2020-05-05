cmake_minimum_required(VERSION 3.8)

project(SPS-Communication VERSION 0.2 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

if(WIN32)
	set(LIBNODAVE_OS "BCCWIN")
elseif(UNIX AND NOT APPLE)
	set(LIBNODAVE_OS "LINUX")
endif()

set(CMAKE_CXX_FLAGS "-D${LIBNODAVE_OS}")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 17)

add_executable(SPS_Com src/main.cpp)

target_include_directories(SPS_Com PRIVATE include)
target_include_directories(SPS_Com PRIVATE extern/libnodave/include)
target_include_directories(SPS_Com PRIVATE extern/asio/include)
target_include_directories(SPS_Com PRIVATE extern/rapidjson/include)

if(UNIX AND NOT APPLE)
	target_link_libraries(SPS_Com nodave)
	target_link_libraries(SPS_Com pthread)
elseif(WIN32)
	target_link_directories(SPS_Com PRIVATE win32)
	target_link_libraries(SPS_Com libnodave)
endif()
