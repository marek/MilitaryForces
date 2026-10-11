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
    foreach(LIB IN LISTS MF_BUNDLED_LIBS)
        install(PROGRAMS $<TARGET_FILE_DIR:${CLIENT_NAME}>/${LIB} DESTINATION .)
    endforeach()
endif()
install(PROGRAMS $<TARGET_FILE:${DEDICATED_NAME}> DESTINATION .)

# paks that may not be distributed (q3_*) never go into a package, even when they were built locally
install(DIRECTORY ${MF_DATA_DIR}/ DESTINATION mfdata OPTIONAL
    FILES_MATCHING PATTERN "*.pk3" PATTERN "q3_*" EXCLUDE)

# Linux also gets a .deb: everything in /opt/militaryforces, and symlinks to it for the commands in /usr/bin and the
# entry in the applications menu; those only go into the .deb, the .zip runs from its own folder
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DEB_OPT /opt/militaryforces)
    set(DEB_LINKS ${CMAKE_BINARY_DIR}/deb_links)
    file(MAKE_DIRECTORY ${DEB_LINKS})
    file(CREATE_LINK ${DEB_OPT}/${CLIENT_NAME} ${DEB_LINKS}/militaryforces SYMBOLIC)
    file(CREATE_LINK ${DEB_OPT}/${DEDICATED_NAME} ${DEB_LINKS}/militaryforces-server SYMBOLIC)
    file(CREATE_LINK ${DEB_OPT}/militaryforces.desktop ${DEB_LINKS}/militaryforces.desktop SYMBOLIC)
    file(CREATE_LINK ${DEB_OPT}/militaryforces.png ${DEB_LINKS}/militaryforces.png SYMBOLIC)

    install(FILES ${CMAKE_SOURCE_DIR}/misc/linux/militaryforces.desktop ${CMAKE_SOURCE_DIR}/misc/linux/militaryforces.png
        DESTINATION . COMPONENT deb)
    install(FILES ${DEB_LINKS}/militaryforces ${DEB_LINKS}/militaryforces-server DESTINATION /usr/bin COMPONENT deb)
    install(FILES ${DEB_LINKS}/militaryforces.desktop DESTINATION /usr/share/applications COMPONENT deb)
    install(FILES ${DEB_LINKS}/militaryforces.png DESTINATION /usr/share/icons/hicolor/64x64/apps COMPONENT deb)
endif()

set(CPACK_PACKAGE_NAME militaryforces)
set(CPACK_PACKAGE_FILE_NAME militaryforces-${PROJECT_VERSION}-${MF_PLATFORM})
set(CPACK_PACKAGE_VENDOR "Military Forces")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Military Forces: air, land and sea combat")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "ZIP;DEB")
else()
    set(CPACK_GENERATOR ZIP)
endif()

set(CPACK_DEBIAN_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}.deb)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marek Kudlacz <marek@kudlacz.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION games)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/marek/MilitaryForces")
# what the binaries need from the system is found from them (the libraries beside the game are left out); curl is
# opened at run time, for downloads from servers
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS_PRIVATE_DIRS ${CMAKE_BINARY_DIR})
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "libcurl4")
# the .deb-only files are left out of the .zip by package_generator.cmake
set(CPACK_COMPONENTS_ALL Unspecified deb)
set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_SOURCE_DIR}/cmake/utils/package_generator.cmake)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_MF_DIST_DIR ${CMAKE_SOURCE_DIR}/dist)
set(CPACK_MF_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(CPACK_POST_BUILD_SCRIPTS ${CMAKE_SOURCE_DIR}/cmake/utils/collect_dist.cmake)
include(CPack)
