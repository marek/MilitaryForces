option(USE_FREETYPE "Freetype font rendering" ON)

if(NOT USE_FREETYPE)
    return()
endif()

find_package(Freetype)

if(NOT FREETYPE_FOUND)
    message(STATUS "FreeType not found, fonts use the pre-rendered font images only")
    return()
endif()

list(APPEND CLIENT_DEFINITIONS BUILD_FREETYPE)
list(APPEND CLIENT_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
list(APPEND CLIENT_LIBRARIES ${FREETYPE_LIBRARIES})
