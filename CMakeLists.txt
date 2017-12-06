########################################################################
# Project setup for LMS7 driver
#
# This project builds the sources into a static library
# and had install rules for the library and headers
########################################################################
cmake_minimum_required(VERSION 2.8)
project(LMS7002MDriver C)


# Set the version information here
set(MAJOR_VERSION 0)
set(API_COMPAT    0)
set(MINOR_VERSION 1)
set(MAINT_VERSION git)

set(LIBVER  "${MAJOR_VERSION}.${API_COMPAT}.${MINOR_VERSION}")

# determine target architecture
execute_process(COMMAND ${CMAKE_C_COMPILER} -print-multiarch OUTPUT_VARIABLE CC_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
if(CC_ARCH MATCHES "i386")
	message(STATUS "Building for i386")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
	set(ARCH "x86")
elseif(CC_ARCH MATCHES "x86_64")
	message(STATUS "Building for x86_64")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
	set(ARCH "x86")
elseif(CC_ARCH MATCHES "aarch64")
	message(STATUS "Building for aarch64")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
	set(ARCH "arm")
elseif(CC_ARCH MATCHES "arm.*eabihf")
	message(STATUS "Building for armhf")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
	set(ARCH "arm")
else()
	message(WARNING "Unknown target architecture ${CC_ARCH}, forcing to FORCE_ARCH=${FORCE_ARCH} variable if it was provided")
	set(ARCH ${FORCE_ARCH})
endif()

set(CROSS_COMPILE_INCLUDE_PATH "/usr/${CC_ARCH}/include")
set(CROSS_COMPILE_LIB_PATH     "/usr/${CC_ARCH}/lib")

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

########################################################################
# package generator
########################################################################
if(NOT CPACK_GENERATOR)
	set(CPACK_GENERATOR DEB)
endif()
set(CPACK_PACKAGE_NAME                "lib${LIB_LMS7002M_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LMS7002MDriver library")
set(CPACK_PACKAGE_VENDOR              "Fairwaves, Inc.")
set(CPACK_PACKAGE_CONTACT             "http://fairwaves.co/wp/contact-us/")
set(CPACK_PACKAGE_VERSION             ${LIBVER}-1)
set(CPACK_DEBIAN_PACKAGE_DEPENDS      "libc6 (>= 2.17)")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
set(CPACK_SET_DESTDIR               "")
set(CPACK_PACKAGING_INSTALL_PREFIX  "/usr")
set(CPACK_PACKAGE_CONTACT           "Sergey Kostanbaev <sergey.kostanbaev@fairwaves.co>")

include(CPack) #include last
