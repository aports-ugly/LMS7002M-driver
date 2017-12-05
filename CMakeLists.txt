########################################################################
# Project setup for LMS7 driver
#
# This project builds the sources into a static library
# and had install rules for the library and headers
########################################################################
cmake_minimum_required(VERSION 2.8)
project(LMS7002MDriver C)

########################################################################
# Compiler setup
########################################################################
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
    add_definitions(-D_GNU_SOURCE)
    add_definitions(-Wall)
    add_definitions(-Wextra)
    add_definitions(-pedantic)
endif()

########################################################################
# Build library
# Defaults to static, set BUILD_SHARED_LIBS=ON for shared
########################################################################
include_directories(include)
file(GLOB LMS7002M_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/LMS7002M_*.c")

if(NOT DEFINED LIB_LMS7002M_NAME)
    set(LIB_LMS7002M_NAME LMS7002M)
endif(NOT DEFINED LIB_LMS7002M_NAME)

add_library(${LIB_LMS7002M_NAME} ${LMS7002M_SOURCES})
install(TARGETS ${LIB_LMS7002M_NAME} DESTINATION lib${LIB_SUFFIX})
set_property(TARGET ${LIB_LMS7002M_NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)

########################################################################
# install headers
########################################################################
if(NOT DEFINED DONT_INSTALL_LMS7002M_INCLUDE)
    install(DIRECTORY include/LMS7002M DESTINATION include)
endif(NOT DEFINED DONT_INSTALL_LMS7002M_INCLUDE)

