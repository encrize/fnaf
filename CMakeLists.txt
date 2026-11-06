cmake_minimum_required(VERSION 3.20)
project(FNAF_CPP LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Default to Release build for optimal performance and size
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler optimization & size reduction flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /GL)
        add_link_options(/LTCG)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -ffunction-sections -fdata-sections)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG -flto)
        add_link_options(-flto -Wl,--gc-sections -s)
    endif()
endif()

# Static linking for Windows builds (no external DLL dependencies)
if(WIN32 OR MINGW)
    add_link_options(-static)
endif()

# Build optimized static raylib from thirdparty/raylib with minimal modules/formats
set(PLATFORM "Desktop" CACHE STRING "Platform target" FORCE)
set(GRAPHICS "GRAPHICS_API_OPENGL_33" CACHE STRING "Graphics target" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build raylib static" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/raylib EXCLUDE_FROM_ALL)
target_compile_definitions(raylib PRIVATE SUPPORT_FILEFORMAT_PNG SUPPORT_FILEFORMAT_JPG)

# Bundle game assets to embed directly into the executable (100% standalone single-file exe)
set(ASSETS_BUNDLE "${CMAKE_CURRENT_BINARY_DIR}/assets.bundle")
add_custom_command(
    OUTPUT ${ASSETS_BUNDLE}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/pack_assets.py
            ${ASSETS_BUNDLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            ${CMAKE_CURRENT_SOURCE_DIR}/config
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/pack_assets.py
    COMMENT "Packing game assets into embedded bundle"
    VERBATIM
)

set_source_files_properties(src/EmbeddedAssets.S PROPERTIES
    OBJECT_DEPENDS ${ASSETS_BUNDLE}
)

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
    src/BundleManager.cpp
    src/EmbeddedAssets.S
    ${ASSETS_BUNDLE}
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
    src/BundleManager.h
)

add_executable(fnaf ${SOURCES} ${HEADERS})

target_include_directories(fnaf PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(fnaf PRIVATE raylib)

if(WIN32 OR MINGW)
    target_link_libraries(fnaf PRIVATE opengl32 gdi32 winmm shell32)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_link_options(fnaf PRIVATE -mwindows)
    endif()
endif()

# Also copy config and assets for convenience in build directory
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
    src/BundleManager.cpp
    src/EmbeddedAssets.S
    ${ASSETS_BUNDLE}
)

add_executable(test_gameplay ${TEST_SOURCES})
target_include_directories(test_gameplay PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_gameplay PRIVATE raylib)

if(WIN32 OR MINGW)
    target_link_libraries(test_gameplay PRIVATE opengl32 gdi32 winmm shell32)
endif()
