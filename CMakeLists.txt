cmake_minimum_required(VERSION 2.6)
project("RTAI Unit Test")

include_directories(
    include
    /usr/realtime/include
)

add_executable(TestRTAI src/TestRTAI.cpp)

target_link_libraries(TestRTAI
    pthread
)
