option(USE_RENDERER_DLOPEN "Dynamically load the renderer(s)" ON)

set(RENDERER_GL2_SOURCES
    tr_animation.cpp tr_backend.cpp tr_bsp.cpp tr_cmds.cpp tr_curve.cpp tr_dsa.cpp
    tr_extramath.cpp tr_extensions.cpp tr_fbo.cpp tr_flares.cpp tr_glsl.cpp tr_image.cpp
    tr_image_dds.cpp tr_init.cpp tr_light.cpp tr_main.cpp tr_marks.cpp tr_mesh.cpp tr_model.cpp
    tr_model_iqm.cpp tr_postprocess.cpp tr_scene.cpp tr_shade.cpp tr_shade_calc.cpp
    tr_shader.cpp tr_shadows.cpp tr_sky.cpp tr_surface.cpp tr_vbo.cpp tr_world.cpp)
list(TRANSFORM RENDERER_GL2_SOURCES PREPEND "${SOURCE_DIR}/renderergl2/")

file(GLOB RENDERER_GL2_SHADER_SOURCES ${SOURCE_DIR}/renderergl2/glsl/*.glsl)

set(SHADERS_DIR ${CMAKE_BINARY_DIR}/shaders.dir)
file(MAKE_DIRECTORY ${SHADERS_DIR})

foreach(SHADER_FILE IN LISTS RENDERER_GL2_SHADER_SOURCES)
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
    set(SHADER_C_FILE ${SHADERS_DIR}/${SHADER_NAME}.cpp)

    add_custom_command(
        OUTPUT ${SHADER_C_FILE}
        COMMAND ${CMAKE_COMMAND}
            -DINPUT_FILE=${SHADER_FILE}
            -DOUTPUT_FILE=${SHADER_C_FILE}
            -DSHADER_NAME=${SHADER_NAME}
            -P ${CMAKE_SOURCE_DIR}/cmake/utils/stringify_shader.cmake
        DEPENDS ${SHADER_FILE}
        COMMENT "Stringify shader ${SHADER_NAME}")

    list(APPEND RENDERER_GL2_SHADER_C_SOURCES ${SHADER_C_FILE})
endforeach()

set(DYNAMIC_RENDERER_SOURCES
    ${SOURCE_DIR}/renderercommon/tr_subs.cpp
    ${SOURCE_DIR}/game/q_shared.cpp
    ${SOURCE_DIR}/game/q_math.cpp)

if(APPLE)
    list(APPEND RENDERER_LIBRARIES "-framework OpenGL" "-framework CoreGraphics")
endif()

function(add_mf_renderer NAME)
    add_library(${NAME} MODULE ${ARGN} ${RENDERER_COMMON_SOURCES} ${SDL_RENDERER_SOURCES} ${DYNAMIC_RENDERER_SOURCES})
    mf_keep_symbols(${NAME})
    set_target_properties(${NAME} PROPERTIES PREFIX "" SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    target_compile_definitions(${NAME} PRIVATE ${CLIENT_DEFINITIONS} USE_RENDERER_DLOPEN
        $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
    target_include_directories(${NAME} PRIVATE ${CLIENT_INCLUDE_DIRS})
    target_link_libraries(${NAME} PRIVATE ${CLIENT_LIBRARIES} ${RENDERER_LIBRARIES})
endfunction()

if(USE_RENDERER_DLOPEN)
    add_mf_renderer(renderer_opengl1 ${RENDERER_GL1_SOURCES})
    add_mf_renderer(renderer_opengl2 ${RENDERER_GL2_SOURCES} ${RENDERER_GL2_SHADER_C_SOURCES})
    list(APPEND CLIENT_DEFINITIONS USE_RENDERER_DLOPEN)
    add_library(renderer_null MODULE ${SOURCE_DIR}/renderernull/tr_null.cpp ${DYNAMIC_RENDERER_SOURCES})
    mf_keep_symbols(renderer_null)
    set_target_properties(renderer_null PROPERTIES PREFIX "" SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    target_compile_definitions(renderer_null PRIVATE ${CLIENT_DEFINITIONS} USE_RENDERER_DLOPEN
        $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
    target_include_directories(renderer_null PRIVATE ${CLIENT_INCLUDE_DIRS})
    target_link_libraries(renderer_null PRIVATE ${CLIENT_LIBRARIES})
    set(MF_RENDERERS renderer_opengl1 renderer_opengl2 renderer_null)
endif()
