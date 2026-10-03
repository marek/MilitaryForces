cmake_minimum_required(VERSION 3.21)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(MilitaryForces VERSION 1.0.9 LANGUAGES C CXX)

set(CLIENT_NAME MilitaryForces)
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/Military Forces/source/code")

set(MF_SANITIZE "" CACHE STRING "Comma separated list of sanitizers to enable, e.g. address,undefined")
option(MF_WARNINGS "Enable extra compiler warnings" OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include(utils/arch)
include(compilers/all)
include(platforms/all)
include(libraries/sdl)
include(libraries/zlib)
include(libraries/jpeg)
include(libraries/openal)
include(libraries/freetype)
include(libraries/ogg)
include(libraries/vorbis)
include(libraries/opus)
include(sources)
include(libraries/curl)
include(renderers)
include(client)
