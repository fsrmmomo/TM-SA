cmake_minimum_required(VERSION 3.20)
project(divSketch)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -msse2 -mssse3 -msse4.1 -msse4.2 -mavx -march=native -fno-asynchronous-unwind-tables")
#set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
set(CMAKE_CXX_FLAGS -pthread)
file(
    GLOB SOURCES src/*.cpp src/*.h
)

#add_executable(div src/div.cpp ${SOURCE_FILES} ${SOURCES})
add_executable(div src/div.cpp ${SOURCES})
target_link_libraries(div rt)
