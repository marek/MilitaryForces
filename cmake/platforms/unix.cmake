if(NOT UNIX)
    return()
endif()

list(APPEND SYSTEM_PLATFORM_SOURCES
    ${SOURCE_DIR}/sys/sys_unix.c
    ${SOURCE_DIR}/sys/con_tty.c)

list(APPEND COMMON_LIBRARIES ${CMAKE_DL_LIBS} m)
