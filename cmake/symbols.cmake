# Release builds are compiled with debug information, which is split into separate files under
# <build>/symbols/<version>-<commit>/ (dSYM on macOS, .debug on Linux, .pdb on Windows), so a crash
# report from a shipped build can be symbolised later. Keep that folder for every release made.
option(MF_KEEP_SYMBOLS "Build release configurations with debug information kept in separate symbol files" ON)

execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE MF_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT MF_COMMIT)
    set(MF_COMMIT unknown)
endif()
set(MF_SYMBOL_DIR ${CMAKE_BINARY_DIR}/symbols/${PROJECT_VERSION}-${MF_COMMIT})

function(mf_keep_symbols TARGET)
    if(NOT MF_KEEP_SYMBOLS)
        return()
    endif()
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/Zi>)
        target_link_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${MF_SYMBOL_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_PDB_FILE:${TARGET}> ${MF_SYMBOL_DIR}
            VERBATIM)
    else()
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-g>)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DFILE=$<TARGET_FILE:${TARGET}> -DOUT=${MF_SYMBOL_DIR} -DCONFIG=$<CONFIG>
                -P ${CMAKE_SOURCE_DIR}/cmake/utils/split_symbols.cmake
            VERBATIM)
    endif()
endfunction()
