if(NOT UNIX OR APPLE)
    return()
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND COMMON_LIBRARIES Threads::Threads)
