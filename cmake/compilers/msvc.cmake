if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    return()
endif()

add_compile_options(/W3 /permissive /Zc:wchar_t /EHsc /MP)
add_compile_options(/wd4018 /wd4244 /wd4267 /wd4305 /wd4996 /wd4800 /wd4530)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE _WINSOCK_DEPRECATED_NO_WARNINGS)

if(MF_SANITIZE)
    add_compile_options(/fsanitize=${MF_SANITIZE})
endif()
