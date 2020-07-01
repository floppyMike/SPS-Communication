cmake_minimum_required(VERSION 3.8)

project(SPS-Communication VERSION 0.4 LANGUAGES CXX)

if(WIN32)
	set(LIBNODAVE_OS "BCCWIN")
elseif(UNIX AND NOT APPLE)
	set(LIBNODAVE_OS "LINUX")
endif()

set(CMAKE_CXX_FLAGS "-D${LIBNODAVE_OS}")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)