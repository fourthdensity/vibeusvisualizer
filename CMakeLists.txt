cmake_minimum_required(VERSION 3.21)
project(Vibeus VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---

# SDL2 (via vcpkg or system)
find_package(SDL2 CONFIG REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)

# Dear ImGui (via vcpkg - core + opengl3 backend; SDL2 backend vendored in src/)
find_package(imgui CONFIG REQUIRED)

# projectM - point to the built library
set(PROJECTM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../projectchilltitties" CACHE PATH "Path to projectM source root")
set(PROJECTM_BUILD "${PROJECTM_ROOT}/build" CACHE PATH "Path to projectM build directory")

# projectM include dirs (source + generated export headers)
set(PROJECTM_INCLUDE_DIRS
    "${PROJECTM_ROOT}/src/api/include"
    "${PROJECTM_BUILD}/src/api/include"          # for generated projectM_export.h
)

# Playlist include dirs (source + generated export headers)
set(PROJECTM_PLAYLIST_INCLUDE_DIRS
    "${PROJECTM_ROOT}/src/playlist/api"
    "${PROJECTM_BUILD}/src/playlist/include"     # for generated projectM_playlist_export.h
)

# projectM libraries
find_library(PROJECTM_LIBRARY
    NAMES projectM-4
    PATHS "${PROJECTM_BUILD}/src/libprojectM/Release"
          "${PROJECTM_BUILD}/src/libprojectM/Debug"
          "${PROJECTM_BUILD}/src/libprojectM"
    NO_DEFAULT_PATH
)

find_library(PROJECTM_PLAYLIST_LIBRARY
    NAMES projectM-4-playlist
    PATHS "${PROJECTM_BUILD}/src/playlist/Release"
          "${PROJECTM_BUILD}/src/playlist/Debug"
          "${PROJECTM_BUILD}/src/playlist"
    NO_DEFAULT_PATH
)

if(NOT PROJECTM_LIBRARY)
    message(FATAL_ERROR "Could not find projectM-4 library. Set PROJECTM_BUILD to the projectM build directory.")
endif()

if(NOT PROJECTM_PLAYLIST_LIBRARY)
    message(FATAL_ERROR "Could not find projectM-4-playlist library. Set PROJECTM_BUILD to the projectM build directory.")
endif()

message(STATUS "projectM library: ${PROJECTM_LIBRARY}")
message(STATUS "projectM playlist library: ${PROJECTM_PLAYLIST_LIBRARY}")

# --- Sources ---
set(SOURCES
    src/main.cpp
    src/audio_capture.cpp
    src/preset_manager.cpp
    src/menu_overlay.cpp
    src/imgui_impl_sdl2.cpp
)

set(HEADERS
    src/audio_capture.h
    src/preset_manager.h
    src/menu_overlay.h
    src/imgui_impl_sdl2.h
)

# --- Executable ---
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECTM_INCLUDE_DIRS}
    ${PROJECTM_PLAYLIST_INCLUDE_DIRS}
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECTM_LIBRARY}
    ${PROJECTM_PLAYLIST_LIBRARY}
    SDL2::SDL2
    SDL2::SDL2main
    OpenGL::GL
    imgui::imgui
)

# Windows-specific: WASAPI audio loopback
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WASAPI_LOOPBACK)
    target_link_libraries(${PROJECT_NAME} PRIVATE ole32 avrt)
endif()

# --- Post-build: copy DLLs ---
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECTM_BUILD}/src/libprojectM/$<CONFIG>/projectM-4.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECTM_BUILD}/src/playlist/$<CONFIG>/projectM-4-playlist.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying projectM DLLs to output directory"
)

# Install target (basic)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
