# CMake generated Testfile for 
# Source directory: /home/xuyang/base
# Build directory: /home/xuyang/base/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(inet_address_test "/home/xuyang/base/build/inet_address_test")
set_tests_properties(inet_address_test PROPERTIES  _BACKTRACE_TRIPLES "/home/xuyang/base/CMakeLists.txt;38;add_test;/home/xuyang/base/CMakeLists.txt;0;")
add_test(work_thread_test "/home/xuyang/base/build/work_thread_test")
set_tests_properties(work_thread_test PROPERTIES  _BACKTRACE_TRIPLES "/home/xuyang/base/CMakeLists.txt;42;add_test;/home/xuyang/base/CMakeLists.txt;0;")
add_test(buffer_test "/home/xuyang/base/build/buffer_test")
set_tests_properties(buffer_test PROPERTIES  _BACKTRACE_TRIPLES "/home/xuyang/base/CMakeLists.txt;46;add_test;/home/xuyang/base/CMakeLists.txt;0;")
add_test(socket_test "/home/xuyang/base/build/socket_test")
set_tests_properties(socket_test PROPERTIES  _BACKTRACE_TRIPLES "/home/xuyang/base/CMakeLists.txt;50;add_test;/home/xuyang/base/CMakeLists.txt;0;")
add_test(event_loop_test "/home/xuyang/base/build/event_loop_test")
set_tests_properties(event_loop_test PROPERTIES  _BACKTRACE_TRIPLES "/home/xuyang/base/CMakeLists.txt;54;add_test;/home/xuyang/base/CMakeLists.txt;0;")
subdirs("third_party/google_test")
