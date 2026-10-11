if(NOT UNIX OR APPLE)
    return()
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND COMMON_LIBRARIES Threads::Threads)

# the game, the server and the renderers look for their libraries (SDL2, FreeType and OpenAL Soft, from
# misc/linux/build-deps.sh) beside themselves, and carry the C++ runtime inside, so from the player's system they need
# only the C library and OpenGL
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
add_link_options(-static-libstdc++ -static-libgcc)
