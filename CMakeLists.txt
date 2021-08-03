cmake_minimum_required(VERSION 3.0)

# set the project name
project(RNDR-Engine VERSION 1.0)
include(GNUInstallDirs)

enable_language(C ASM)
enable_testing()

# set required
find_package(OpenSSL REQUIRED)
find_package(Perl REQUIRED)

if (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
  message( FATAL_ERROR "Target processor is not 64 Bit Arm (aarch64)" )
endif ()

# include dirs
include_directories(${OPENSSL_INCLUDE_DIRS})

set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/arm64cpuid.S
  ${CMAKE_CURRENT_SOURCE_DIR}/arm_arch.h
  ${CMAKE_CURRENT_SOURCE_DIR}/armcap.c
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/arm64cpuid.S
  COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/arm64cpuid.pl > ${CMAKE_CURRENT_SOURCE_DIR}/arm64cpuid.S
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/arm64cpuid.pl ${CMAKE_CURRENT_SOURCE_DIR}/arm-xlate.pl
  VERBATIM)

# generate library
add_library(eng_rndr SHARED ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/eng_rndr.c)
set_target_properties(eng_rndr PROPERTIES
  SOVERSION 0.0.0
  POSITION_INDEPENDENT_CODE ON
)

# generate test
add_executable(rndr_sanitytest ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/rndr_sanitytest.c)
add_test(NAME rndr_rndrrs_test COMMAND rndr_sanitytest)

# set install location
install(
  TARGETS eng_rndr
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
