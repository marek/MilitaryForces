option(USE_MUMBLE "Mumble support" ON)
if(USE_MUMBLE)
    list(APPEND CLIENT_DEFINITIONS USE_MUMBLE)
endif()

set(MF_LIBRARIES game cgame ui)
if(NOT USE_RENDERER_DLOPEN)
    list(APPEND MF_LIBRARIES renderer)
endif()

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

# Linux: the libraries made by misc/linux/build-deps.sh are copied beside the executable (OpenAL Soft too: the game
# opens it by name, and the copy is found first through the executable's $ORIGIN search path). Without them the
# system's own are used
set(MF_BUNDLED_LIBS "")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    foreach(LIB libSDL2-2.0.so.0 libfreetype.so.6 libopenal.so.1)
        unset(MF_LIB_FILE CACHE)
        find_file(MF_LIB_FILE ${LIB} PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH)
        if(MF_LIB_FILE)
            add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${MF_LIB_FILE} $<TARGET_FILE_DIR:${CLIENT_NAME}>/${LIB}
                VERBATIM)
            list(APPEND MF_BUNDLED_LIBS ${LIB})
        endif()
    endforeach()
    unset(MF_LIB_FILE CACHE)
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
    # SDL2, FreeType and anything else that is not part of macOS is copied beside the executable
    option(MF_BUNDLE_LIBS "Copy the dynamic libraries the game needs into the app" ON)
    # OpenAL Soft is opened by name at run time, so nothing links to it and the step below would not find it: it is
    # copied here when the dependency build has made it (misc/macos/build-deps.sh). Without it the game uses Apple's
    if(MF_BUNDLE_LIBS)
        set(MF_OPENAL_SOFT "")
        foreach(PREFIX IN LISTS CMAKE_PREFIX_PATH)
            if(NOT MF_OPENAL_SOFT AND EXISTS "${PREFIX}/lib/libopenal.1.dylib")
                set(MF_OPENAL_SOFT "${PREFIX}/lib/libopenal.1.dylib")
            endif()
        endforeach()
        if(MF_OPENAL_SOFT)
            add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${MF_OPENAL_SOFT} $<TARGET_FILE_DIR:${CLIENT_NAME}>/libopenal.1.dylib
                COMMAND codesign --force --sign ${MF_SIGN_IDENTITY} $<TARGET_FILE_DIR:${CLIENT_NAME}>/libopenal.1.dylib
                VERBATIM)
        endif()
    endif()
    if(MF_BUNDLE_LIBS)
        set(MF_BUNDLE_SEARCH "")
        foreach(PREFIX IN LISTS CMAKE_PREFIX_PATH)
            list(APPEND MF_BUNDLE_SEARCH "${PREFIX}/lib")
        endforeach()
        list(APPEND MF_BUNDLE_SEARCH /usr/local/lib /opt/homebrew/lib)
        list(JOIN MF_BUNDLE_SEARCH "|" MF_BUNDLE_SEARCH)
        add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DFOLDER=$<TARGET_FILE_DIR:${CLIENT_NAME}> -DSIGN=${MF_SIGN_IDENTITY} -DSEARCH=${MF_BUNDLE_SEARCH}
                -P ${CMAKE_SOURCE_DIR}/cmake/utils/bundle_libs.cmake
            VERBATIM)
        # A renderer rebuilt on its own is copied into the bundle as the linker made it, still looking for SDL2 by
        # its build path: the game then runs with two copies of SDL2 and fails at start. The same fix-up is run for it
        foreach(RENDERER IN LISTS MF_RENDERERS)
            add_custom_command(TARGET ${RENDERER} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -DFOLDER=$<TARGET_FILE_DIR:${CLIENT_NAME}> -DSIGN=${MF_SIGN_IDENTITY} -DSEARCH=${MF_BUNDLE_SEARCH}
                    -P ${CMAKE_SOURCE_DIR}/cmake/utils/bundle_libs.cmake
                VERBATIM)
        endforeach()
    endif()
    add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
        COMMAND codesign --force --deep --sign ${MF_SIGN_IDENTITY} $<TARGET_BUNDLE_DIR:${CLIENT_NAME}>
        VERBATIM)
endif()
