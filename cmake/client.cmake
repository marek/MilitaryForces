set(MF_LIBRARIES vehiclelib game cgame ui)
if(NOT USE_RENDERER_DLOPEN)
    list(APPEND MF_LIBRARIES renderer)
endif()

add_library(vehiclelib OBJECT ${VEHICLELIB_SOURCES})
add_library(game OBJECT ${GAME_SOURCES})
add_library(cgame OBJECT ${CGAME_SOURCES})
add_library(ui OBJECT ${UI_SOURCES})
if(NOT USE_RENDERER_DLOPEN)
    add_library(renderer OBJECT ${RENDERER_SOURCES} ${SDL_RENDERER_SOURCES})
endif()

set(CLIENT_ALL_SOURCES
    ${QCOMMON_SOURCES}
    ${ASM_SOURCES}
    ${SERVER_SOURCES}
    ${CLIENT_SOURCES}
    ${SYSTEM_SOURCES}
    ${SDL_SOURCES}
    ${SYSTEM_PLATFORM_SOURCES}
    ${CLIENT_PLATFORM_SOURCES})

add_executable(${CLIENT_NAME} ${CLIENT_EXECUTABLE_OPTIONS} ${CLIENT_ALL_SOURCES})

foreach(TARGET IN LISTS MF_LIBRARIES ITEMS ${CLIENT_NAME})
    target_compile_definitions(${TARGET} PRIVATE ${CLIENT_DEFINITIONS} $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
    target_include_directories(${TARGET} PRIVATE ${CLIENT_INCLUDE_DIRS})
    target_link_libraries(${TARGET} PRIVATE ${CLIENT_LIBRARIES} ${RENDERER_LIBRARIES})
endforeach()

target_link_libraries(${CLIENT_NAME} PRIVATE ${MF_LIBRARIES}
    ${CLIENT_LIBRARIES} ${RENDERER_LIBRARIES} ${COMMON_LIBRARIES})

if(APPLE AND BUILD_MACOS_APP)
    finish_macos_app()
endif()

if(WIN32 AND TARGET SDL2::SDL2)
    add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:${CLIENT_NAME}>)
endif()

foreach(RENDERER IN LISTS MF_RENDERERS)
    add_dependencies(${CLIENT_NAME} ${RENDERER})
    add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RENDERER}> $<TARGET_FILE_DIR:${CLIENT_NAME}>)
endforeach()
