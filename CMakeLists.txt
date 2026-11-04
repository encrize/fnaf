cmake_minimum_required(VERSION 3.20)
project(FNAF_CPP LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release build for optimal performance and size
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Platform / Compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -ffunction-sections -fdata-sections)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG -flto)
        add_link_options(-flto -Wl,--gc-sections -s)
    endif()
endif()

# Build optimized static raylib from thirdparty/raylib with minimal modules/formats
set(PLATFORM "Desktop" CACHE STRING "Platform target" FORCE)
set(GRAPHICS "GRAPHICS_API_OPENGL_33" CACHE STRING "Graphics target" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build raylib static" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/raylib EXCLUDE_FROM_ALL)

set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Office.cpp
    src/Cameras.cpp
    src/Animatronic.cpp
    src/Power.cpp
    src/UI.cpp
    src/Audio.cpp
    src/AssetManager.cpp
    src/NightManager.cpp
    src/Settings.cpp
)

set(HEADERS
    src/Common.h
    src/Game.h
    src/Office.h
    src/Cameras.h
    src/Animatronic.h
    src/Power.h
    src/UI.h
    src/Audio.h
    src/AssetManager.h
    src/NightManager.h
    src/Settings.h
)

add_executable(fnaf ${SOURCES} ${HEADERS})

target_include_directories(fnaf PRIVATE src)
target_link_libraries(fnaf PRIVATE raylib)

# Ensure config and assets directories are accessible from build directory
add_custom_command(TARGET fnaf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:fnaf>/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:fnaf>/assets
    COMMENT "Copying assets and config to binary folder"
)

# Test executable
set(TEST_SOURCES
    tests/test_gameplay.cpp
    src/Settings.cpp
    src/NightManager.cpp
    src/Power.cpp
    src/Animatronic.cpp
    src/Office.cpp
    src/Cameras.cpp
    src/Audio.cpp
    src/AssetManager.cpp
)

add_executable(test_gameplay ${TEST_SOURCES})
target_include_directories(test_gameplay PRIVATE src)
target_link_libraries(test_gameplay PRIVATE raylib)

