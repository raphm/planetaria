cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_policy(SET CMP0054 NEW)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

if( CMAKE_SYSTEM_NAME MATCHES "Windows" )
  SET(CL_COVERAGE_COMPILE_FLAGS "-D_CRT_SECURE_NO_WARNINGS -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING")
  SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CL_COVERAGE_COMPILE_FLAGS} " )
  # SET(CL_COVERAGE_LINK_FLAGS    "/NODEFAULTLIB:LIBCMT  /SUBSYSTEM:WINDOWS")
endif()


project(planetaria)

file(GLOB SRC_FILES src/*.cpp)
add_executable(planetaria ${SRC_FILES})
set_property(TARGET planetaria PROPERTY CXX_STANDARD 17)
target_link_libraries(planetaria novas-wrapper) 

include_directories(./src/)
include_directories(../novas-wrapper/src/)
include_directories(../novas-wrapper/NOVAS-C/Cdist/)

add_custom_command(TARGET planetaria POST_BUILD
           COMMAND ${CMAKE_COMMAND} -E copy_directory
           ${CMAKE_SOURCE_DIR}/ephemeris-data $<TARGET_FILE_DIR:planetaria>/data)
