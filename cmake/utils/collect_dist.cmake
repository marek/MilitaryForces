# run by cpack after it has made the archives: copies them to dist/ and rewrites dist/RELEASE_NOTES.md
file(MAKE_DIRECTORY ${CPACK_MF_DIST_DIR})
foreach(PACKAGE IN LISTS CPACK_PACKAGE_FILES)
    file(COPY ${PACKAGE} DESTINATION ${CPACK_MF_DIST_DIR})
endforeach()
set(DIST ${CPACK_MF_DIST_DIR})
set(TAG v${CPACK_PACKAGE_VERSION})
include(${CPACK_MF_SOURCE_DIR}/cmake/utils/release_notes.cmake)
