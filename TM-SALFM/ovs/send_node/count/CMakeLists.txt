cmake_minimum_required(VERSION 3.20)
project(countFlow)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -msse2 -mssse3 -msse4.1 -msse4.2 -mavx -march=native -fno-asynchronous-unwind-tables")
#set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
set(CMAKE_CXX_FLAGS -pthread)
#link_libraries(-lwsock32)
#set(CMAKE_CXX_FLAGS -Iphlpapi)
file(
        #        GLOB SOURCES src/*.cpp src/*.h
        GLOB SOURCES src/*.cpp src/*.h
)

#add_executable(div src/div.cpp ${SOURCE_FILES} ${SOURCES})
add_executable(count src/count.cpp ${SOURCES})
#add_executable(main main.cpp ${SOURCES})
target_link_libraries(count rt)
