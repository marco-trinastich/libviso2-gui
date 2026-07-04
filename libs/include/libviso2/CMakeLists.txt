# Project info
cmake_minimum_required(VERSION 3.10)
# Target platform (Win32 / x64 / ARM64) is selected at configure time via -A.
project(libviso2 VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories for library artifacts
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Source files
file(GLOB SOURCES 
    "*.cpp" 
    "*.h"
)
# Header files (for installation)
file(GLOB HEADERS "*.h")

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Set up DLL exports (for Windows)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DLIBVISO2_EXPORTS)
    target_compile_definitions(${PROJECT_NAME} INTERFACE -DLIBVISO2_IMPORTS)
endif()

# Export symbols for Windows DLL
set_target_properties(${PROJECT_NAME} PROPERTIES 
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    DEFINE_SYMBOL "LIBVISO2_EXPORTS"
)

# Installation rules
install(TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(FILES ${HEADERS} DESTINATION include/libviso2)

# Suppress warnings (e.g. possible loss of data, float to int conversions)
target_compile_options(${PROJECT_NAME} PRIVATE "/wd4244")

# On ARM64 the SSE intrinsics are bridged to NEON via sse2neon.h, which requires
# MSVC's conforming preprocessor.
if(CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
    target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:preprocessor")
endif()