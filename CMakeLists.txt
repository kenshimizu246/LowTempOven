
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

file(GLOB_RECURSE SOURCE_FILES "*.cpp"  )

message(${SOURCE_FILES})
message("CMAKE_CXX_COMPILER:" ${CMAKE_CXX_COMPILER})

add_definitions(" -g -Wall -std=c++11 ")

include_directories(
	/usr/local/include
	/usr/include
	/usr/lib64
)

add_executable(ovnctrl
src/Config.cpp
src/App.cpp
src/Message.cpp
src/Controller.cpp
src/worker/max31855_worker.cpp
	)
target_link_libraries(Alexo
	pthread
	wiringPi
	uuid
        boost_system
	)



