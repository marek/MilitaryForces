if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$")
    return()
endif()

set(ASM_SOURCES
    ${SOURCE_DIR}/asm/ftola.cpp
    ${SOURCE_DIR}/asm/snapvector.cpp
)

add_compile_options(-fno-strict-aliasing -fwrapv -pipe -fno-operator-names -Wno-narrowing)

if(MF_WARNINGS)
    add_compile_options(-Wall -Wextra -Wuninitialized -Wno-unused-parameter -Wno-sign-compare
        -Wno-missing-field-initializers -Wno-unused-variable -Wno-unused-function)
else()
    add_compile_options(-w)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fpermissive)
else()
    add_compile_options(-ferror-limit=0)
endif()

if(MF_SANITIZE)
    add_compile_options(-fsanitize=${MF_SANITIZE} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${MF_SANITIZE})
endif()
