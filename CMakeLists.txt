cmake_minimum_required(VERSION 3.25)

project(CONNTRACE VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT WIN32)
	message(FATAL_ERROR "CONNTRACE requires Windows.")
endif()

add_library(conntrace_core STATIC
	src/tcp.c
)
target_include_directories(conntrace_core PUBLIC include)
target_compile_definitions(conntrace_core PUBLIC WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
target_link_libraries(conntrace_core PUBLIC iphlpapi ws2_32)

add_executable(conntrace src/main.c)
target_link_libraries(conntrace PRIVATE conntrace_core)

if(MSVC)
	target_compile_options(conntrace_core PRIVATE /W4 /WX)
	target_compile_options(conntrace PRIVATE /W4 /WX)
else()
	message(FATAL_ERROR "CONNTRACE v0.1 is configured for the MSVC C compiler.")
endif()

include(CTest)
if(BUILD_TESTING)
	add_executable(conntrace_unit_tests tests/tcp_tests.c)
	target_link_libraries(conntrace_unit_tests PRIVATE conntrace_core)
	target_compile_options(conntrace_unit_tests PRIVATE /W4 /WX)
	add_test(NAME tcp_state_names COMMAND conntrace_unit_tests)
	add_test(NAME tcp_snapshot_cli COMMAND conntrace --once)
endif()
