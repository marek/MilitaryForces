# read by cpack once for each generator
if(CPACK_GENERATOR STREQUAL "DEB")
    set(CPACK_PACKAGING_INSTALL_PREFIX /opt/militaryforces)
else()
    # the archives take every component but the .deb's own files, all in one archive
    list(REMOVE_ITEM CPACK_COMPONENTS_ALL deb)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
    set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY ON)
endif()
