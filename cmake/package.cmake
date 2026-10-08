# cpack writes militaryforces-<version>-<platform> archives into dist/: the game, the dedicated server
# and the game paks, laid out so the folder runs as it is.
set(MF_DATA_DIR "${CMAKE_SOURCE_DIR}/../assets/build/mfdata" CACHE PATH "Folder with the built game paks (assets repository, build.py)")

if(NOT MF_PLATFORM)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" MF_PLATFORM)
    if(NOT WIN32)
        if(ARCH STREQUAL "arm64")
            string(APPEND MF_PLATFORM "-arm")
        else()
            string(APPEND MF_PLATFORM "-x86")
        endif()
    endif()
endif()
set(MF_PLATFORM "${MF_PLATFORM}" CACHE STRING "Platform name used for the package file")

# plain copies: nothing is relinked, so the signatures made by the build stay valid
if(APPLE AND BUILD_MACOS_APP)
    install(DIRECTORY $<TARGET_BUNDLE_DIR:${CLIENT_NAME}> DESTINATION . USE_SOURCE_PERMISSIONS)
else()
    install(PROGRAMS $<TARGET_FILE:${CLIENT_NAME}> DESTINATION .)
    foreach(RENDERER IN LISTS MF_RENDERERS)
        install(PROGRAMS $<TARGET_FILE:${RENDERER}> DESTINATION .)
    endforeach()
    if(WIN32 AND TARGET SDL2::SDL2)
        install(PROGRAMS $<TARGET_FILE:SDL2::SDL2> DESTINATION .)
    endif()
endif()
install(PROGRAMS $<TARGET_FILE:${DEDICATED_NAME}> DESTINATION .)

# paks that may not be distributed (q3_*) never go into a package, even when they were built locally
install(DIRECTORY ${MF_DATA_DIR}/ DESTINATION mfdata OPTIONAL
    FILES_MATCHING PATTERN "*.pk3" PATTERN "q3_*" EXCLUDE)

set(CPACK_PACKAGE_NAME militaryforces)
set(CPACK_PACKAGE_FILE_NAME militaryforces-${PROJECT_VERSION}-${MF_PLATFORM})
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "TGZ;ZIP")
else()
    set(CPACK_GENERATOR ZIP)
endif()
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_MF_DIST_DIR ${CMAKE_SOURCE_DIR}/dist)
set(CPACK_MF_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(CPACK_POST_BUILD_SCRIPTS ${CMAKE_SOURCE_DIR}/cmake/utils/collect_dist.cmake)
include(CPack)
