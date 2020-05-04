# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.1)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(SPS-Communication VERSION 0.2 LANGUAGES CXX)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)
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

# Adding something we can run - Output name matches target name
add_executable(SPS_Com src/main.cpp)

target_include_directories(SPS_Com PRIVATE include)
target_include_directories(SPS_Com PRIVATE extern/libnodave/include)
target_include_directories(SPS_Com PRIVATE extern/asio/include)
target_include_directories(SPS_Com PRIVATE extern/rapidjson/include)

if(UNIX AND NOT APPLE)
	target_link_libraries(SPS_Com nodave)
elseif(WIN32)
	target_link_directories(SPS_Com win32)
	target_link_libraries(SPS_Com libnodave)
endif()
