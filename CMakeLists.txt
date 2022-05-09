cmake_minimum_required(VERSION 3.0.0)
project(base VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

add_library(base)
target_include_directories(base PUBLIC base/)
target_sources(base PRIVATE
    base/io/epoller.h
    base/io/io_base.h
    base/io/io_object.h

    base/work_thread.h
    base/buffer.h 
    base/inet_address.cpp 
    base/socket.cpp
    base/udp_server.h
    base/event_loop.h
    base/event_loop.cpp
    
    base/hmac_sha256.h
    )
target_link_libraries(base PRIVATE pthread ssl crypto)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

include(CTest)
enable_testing()
add_subdirectory(third_party/google_test)
include_directories(third_party/google_test/googletest/include)

add_executable(inet_address_test base/inet_address_test.cpp)
target_link_libraries(inet_address_test base gtest gtest_main)
add_test(NAME inet_address_test COMMAND inet_address_test)

add_executable(work_thread_test base/work_thread_test.cpp)
target_link_libraries(work_thread_test base gtest gtest_main)
add_test(NAME work_thread_test COMMAND work_thread_test)

add_executable(buffer_test base/buffer_test.cpp)
target_link_libraries(buffer_test base gtest gtest_main)
add_test(NAME buffer_test COMMAND buffer_test)

add_executable(socket_test base/socket_test.cpp)
target_link_libraries(socket_test base gtest gtest_main)
add_test(NAME socket_test COMMAND socket_test)

add_executable(event_loop_test base/event_loop_test.cpp)
target_link_libraries(event_loop_test base gtest gtest_main)
add_test(NAME event_loop_test COMMAND event_loop_test)

# add_executable(udp_server_test base/udp_server_test.cpp)
# target_link_libraries(udp_server_test punch_hole gtest gtest_main)
# add_test(NAME udp_server_test COMMAND udp_server_test)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
