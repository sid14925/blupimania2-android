# Android native build: SDL2 + the BlupiMania 2 game library.

cmake_minimum_required(VERSION 3.16)
project(blupimania2-android)

# repo root = android/app/jni/../../..
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
# SDL2 source checkout sits next to the repo
get_filename_component(SDL2_DIR "${REPO_ROOT}/../SDL2-src" ABSOLUTE)

add_subdirectory(${SDL2_DIR} sdl2-build)
add_subdirectory(${REPO_ROOT} game-build)
