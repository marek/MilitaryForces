if(NOT WIN32)
    return()
endif()

list(APPEND SYSTEM_PLATFORM_SOURCES
    ${SOURCE_DIR}/sys/sys_win32.cpp
    ${SOURCE_DIR}/sys/con_passive.cpp
    ${SOURCE_DIR}/win32/winquake.rc)

list(APPEND COMMON_LIBRARIES ws2_32 winmm psapi)

if(MINGW)
    list(APPEND COMMON_LIBRARIES mingw32)
    add_link_options(-static-libgcc -static-libstdc++)
    string(APPEND CMAKE_CXX_STANDARD_LIBRARIES " -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive,-Bdynamic")
endif()

list(APPEND CLIENT_INCLUDE_DIRS ${SOURCE_DIR}/win32)

set(CLIENT_EXECUTABLE_OPTIONS WIN32)
