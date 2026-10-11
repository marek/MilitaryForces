# read by cpack once for each generator
if(CPACK_GENERATOR STREQUAL "DEB")
    set(CPACK_PACKAGING_INSTALL_PREFIX /opt/militaryforces)
else()
    list(REMOVE_ITEM CPACK_COMPONENTS_ALL deb)
endif()
