# Project info
cmake_minimum_required(VERSION 3.10)
# Target platform (Win32 / x64 / ARM64) is chosen at configure time via -A,
# NOT hardcoded here. See .vscode/scripts/cmake-configure.ps1 (-Platform).
project(libviso2gui VERSION 1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependency artifacts are organized per target platform under libs/bin|lib/<platform>.
# CMAKE_VS_PLATFORM_NAME reflects the -A value chosen at configure time (Win32/x64/ARM64).
set(DEP_PLATFORM "${CMAKE_VS_PLATFORM_NAME}")
if(NOT DEP_PLATFORM)
    set(DEP_PLATFORM "Win32") # fallback for non-Visual Studio generators
endif()
set(DEP_BIN "${CMAKE_CURRENT_SOURCE_DIR}/libs/bin/${DEP_PLATFORM}")
set(DEP_LIB "${CMAKE_CURRENT_SOURCE_DIR}/libs/lib/${DEP_PLATFORM}")
message(STATUS "LibViso2 GUI: dependency artifacts for platform '${DEP_PLATFORM}' -> ${DEP_BIN}")

# Dependencies

# ZLIB
add_library(zlib SHARED IMPORTED)
set_target_properties(zlib PROPERTIES
    IMPORTED_LOCATION "${DEP_BIN}/zlib.dll"
    IMPORTED_IMPLIB "${DEP_LIB}/zlib.lib"
)
target_include_directories(zlib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/libs/include/zlib")

# PNG
add_library(png SHARED IMPORTED)
set_target_properties(png PROPERTIES
    IMPORTED_LOCATION "${DEP_BIN}/libpng14.dll"
    IMPORTED_IMPLIB "${DEP_LIB}/libpng14.lib"
)
target_include_directories(png INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/libs/include/png")

# PNG++
add_library(png++ INTERFACE)
target_include_directories(png++ INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/libs/include/png++")

# LibViso2
add_library(libviso2 SHARED IMPORTED)
set_target_properties(libviso2 PROPERTIES
    IMPORTED_LOCATION "${DEP_BIN}/libviso2.dll"
    IMPORTED_IMPLIB "${DEP_LIB}/libviso2.lib"
)
target_include_directories(libviso2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/libs/include/libviso2")

# Source files
file(GLOB_RECURSE PROJECT_SRC_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Create program
add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SRC_FILES})

# Include directories
set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
function(include_directories_recursive dir)
    file(GLOB_RECURSE ALL_SUBDIRS LIST_DIRECTORIES TRUE RELATIVE ${dir} ${dir}/*)
    foreach(subdir ${ALL_SUBDIRS})
        if(IS_DIRECTORY ${dir}/${subdir})
            target_include_directories(${PROJECT_NAME} PRIVATE ${dir}/${subdir})
        endif()
    endforeach()
endfunction()
include_directories_recursive(${PROJECT_SRC_DIR})

# Link with proper targets
target_link_libraries(${PROJECT_NAME} PRIVATE
    zlib
    png
    png++
    libviso2
)

# Install the platform's dependency DLLs (whatever exists in DEP_BIN).
# All targets link the Universal CRT (shipped with Windows 10/11), so no CRT
# redistributable is bundled.
install(DIRECTORY "${DEP_BIN}/"
    DESTINATION bin
    FILES_MATCHING PATTERN "*.dll"
)

# Copy the platform's dependency DLLs next to the built exe (for development runs)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${DEP_BIN}"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Add version information directly in CMake
if(WIN32)
  # Generate a simple version resource file directly
  file(WRITE "${CMAKE_BINARY_DIR}/version.rc" 
"1 VERSIONINFO
FILEVERSION ${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},0,0
PRODUCTVERSION ${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},0,0
BEGIN
    BLOCK \"StringFileInfo\"
    BEGIN
        BLOCK \"040904E4\"
        BEGIN
            VALUE \"CompanyName\", \"Marco Trinastich\"
            VALUE \"FileDescription\", \"LibViso2 GUI Application\"
            VALUE \"FileVersion\", \"${PROJECT_VERSION}\"
            VALUE \"InternalName\", \"${PROJECT_NAME}\"
            VALUE \"LegalCopyright\", \"Copyright (c) 2025\"
            VALUE \"OriginalFilename\", \"${PROJECT_NAME}.exe\"
            VALUE \"ProductName\", \"${PROJECT_NAME}\"
            VALUE \"ProductVersion\", \"${PROJECT_VERSION}\"
        END
    END
    BLOCK \"VarFileInfo\"
    BEGIN
        VALUE \"Translation\", 0x409, 1252
    END
END")

  # Add the version resource to the executable
  target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/version.rc")
endif()

# Suppress warnings (e.g. possible loss of data, float to int conversions)
target_compile_options(${PROJECT_NAME} PRIVATE "/wd4244")

# The app includes libviso2 headers, which on ARM64 pull in sse2neon.h; that
# header requires MSVC's conforming preprocessor.
if(DEP_PLATFORM STREQUAL "ARM64")
    target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:preprocessor")
endif()