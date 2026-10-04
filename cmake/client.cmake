option(USE_MUMBLE "Mumble support" ON)
if(USE_MUMBLE)
    list(APPEND CLIENT_DEFINITIONS USE_MUMBLE)
endif()

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

mf_keep_symbols(${CLIENT_NAME})


if(WIN32 AND TARGET SDL2::SDL2)
    add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:${CLIENT_NAME}>)
endif()

foreach(RENDERER IN LISTS MF_RENDERERS)
    add_dependencies(${CLIENT_NAME} ${RENDERER})
    # copied next to the client when the renderer itself is rebuilt, not only when the client relinks
    add_custom_command(TARGET ${RENDERER} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${CLIENT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RENDERER}> $<TARGET_FILE_DIR:${CLIENT_NAME}>)
    add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RENDERER}> $<TARGET_FILE_DIR:${CLIENT_NAME}>)
endforeach()

# (after the renderer copies above, so what is signed is what stays in the bundle)
# An ad-hoc signature gives the app bundle a stable identity, so macOS remembers permissions (microphone)
# for a given build instead of asking at every launch. A real signing identity can be set with MF_SIGN_IDENTITY.
if(APPLE AND BUILD_MACOS_APP)
    set(MF_SIGN_IDENTITY "-" CACHE STRING "Code signing identity for the macOS app (- for ad-hoc)")
    # the renderer libraries sit beside the executable, where --deep does not look: sign them first
    foreach(RENDERER IN LISTS MF_RENDERERS)
        add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
            COMMAND codesign --force --sign ${MF_SIGN_IDENTITY} $<TARGET_FILE_DIR:${CLIENT_NAME}>/$<TARGET_FILE_NAME:${RENDERER}>
            VERBATIM)
    endforeach()
    add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
        COMMAND codesign --force --deep --sign ${MF_SIGN_IDENTITY} $<TARGET_BUNDLE_DIR:${CLIENT_NAME}>
        VERBATIM)
endif()
