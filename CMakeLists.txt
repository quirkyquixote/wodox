# The name of our project is "LLR". CMakeLists files in this project can 
# refer to the root source directory of the project as ${LLR_SOURCE_DIR} and 
# to the root binary directory of the project as ${LLR_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.6) 
project (LLR) 

set (CMAKE_C_FLAGS "-g -std=gnu99")
add_definitions(-DDATADIR=\"${CMAKE_INSTALL_PREFIX}/share\")
add_definitions(-DPACKAGE=\"wodox\")
add_definitions(-DVERSION=\"1.1.0\")

set (LLR_SOURCE_DIR "${LLR_SOURCE_DIR}/src")

# Needed for SDL_gfx and Chipmunk
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

find_package(SDL)
find_package(SDL_image)
find_package(SDL_mixer)
find_package(SDL_ttf)
find_package(SDL_gfx)

find_library(M_LIB m)

include_directories (
  ${LLR_SOURCE_DIR}
  ${SDL_INCLUDE_DIR}
  ${SDLIMAGE_INCLUDE_DIR}
  ${SDLMIXER_INCLUDE_DIR}
  ${SDLTTF_INCLUDE_DIR}
  ${SDL_gfx_INCLUDE_DIR}
) 


# Recurse into the "src" and "data" subdirectories. This does not actually 
# cause another cmake executable to run. The same process will walk through 
# the project's entire directory structure. 
add_subdirectory (src) 
add_subdirectory (data) 
