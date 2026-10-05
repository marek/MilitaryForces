set(DEDICATED_NAME MilitaryForcesDed)

set(DEDICATED_SOURCES
    ${QCOMMON_SOURCES}
    ${ASM_SOURCES}
    ${SERVER_SOURCES}
    ${SYSTEM_SOURCES}
    ${SYSTEM_PLATFORM_SOURCES}
    ${SOURCE_DIR}/null/null_client.cpp
    ${VEHICLELIB_SOURCES}
    ${GAME_SOURCES})

add_executable(${DEDICATED_NAME} ${DEDICATED_SOURCES})
target_compile_definitions(${DEDICATED_NAME} PRIVATE ${CLIENT_DEFINITIONS} DEDICATED $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
target_include_directories(${DEDICATED_NAME} PRIVATE ${CLIENT_INCLUDE_DIRS})
target_link_libraries(${DEDICATED_NAME} PRIVATE ${CLIENT_LIBRARIES} ${COMMON_LIBRARIES})
mf_keep_symbols(${DEDICATED_NAME})

# the server shares the client's library list but calls nothing in SDL2 or FreeType: drop libraries it does not
# use, so the server runs on its own without them beside it
if(APPLE)
    target_link_options(${DEDICATED_NAME} PRIVATE LINKER:-dead_strip_dylibs)
endif()
