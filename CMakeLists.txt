cmake_minimum_required(VERSION 3.8)

if (NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "No build type selected")
endif ()

### SETTINGS ###

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif ()

set(CMAKE_CXX_STANDARD 17)

project(LuaPlusPlus)

### SOURCES ###

set(SOURCE_FILES
        src/lapi.cpp
        src/lauxlib.cpp
        src/lbaselib.cpp
        src/lcode.cpp
        src/lcorolib.cpp
        src/lctype.cpp
        src/ldblib.cpp
        src/ldebug.cpp
        src/ldo.cpp
        src/ldump.cpp
        src/lfunc.cpp
        src/lgc.cpp
        src/linit.cpp
        src/liolib.cpp
        src/llex.cpp
        src/lmathlib.cpp
        src/lmem.cpp
        src/loadlib.cpp
        src/lobject.cpp
        src/lopcodes.cpp
        src/loslib.cpp
        src/lparser.cpp
        src/lstate.cpp
        src/lstring.cpp
        src/lstrlib.cpp
        src/ltable.cpp
        src/ltablib.cpp
        src/ltm.cpp
        src/ltype.cpp
        src/lua.cpp
        src/lundump.cpp
        src/lutf8lib.cpp
        src/lvm.cpp
        src/lzio.cpp
        )

set(TEST_SOURCE_FILES
        tests/LuaPrinter.cpp
        tests/Main.cpp
        tests/StringUtil.cpp
        tests/TestBasicPrint.cpp
        tests/TestCommon.cpp
        )

### INCLUDES ###

include_directories(src)
include_directories(SYSTEM libraries)

### DEFINES ###

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG -D_DEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -DDEBUG -D_DEBUG")

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DDEBUG -D_DEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DDEBUG -D_DEBUG")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g1 -DNDEBUG -Wno-error")
set(CMAKE_C_FLAGS_RELEASE "-O3 -g1 -DNDEBUG -Wno-error")

add_definitions(
        -DLLONG_MAX=INT_MAX
        -DLLONG_MIN=INT_MIN
        -DLUA_USE_APICHECK
)

### FLAGS ###

# warnings
add_compile_options(
        -Wall
        -Wextra
        -Wunused
        -Wuninitialized
        -Wno-dangling-else
        -Wformat-security
        -Wno-array-bounds
        -pedantic

        -Wdelete-incomplete

#        -Wshadow
        -Wsign-compare
        -Wundef
        -Wwrite-strings
        -Wredundant-decls
        -Wdisabled-optimization
        -Waggregate-return
#        -Wdouble-promotion
)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(
            -Wno-parentheses
            -Wno-maybe-uninitialized
            -Wimplicit-fallthrough=2
            -Wno-aggressive-loop-optimizations
            -Wno-stringop-overflow
            -Wsuggest-override
    )
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(
            -Wno-logical-op-parentheses
            -Winconsistent-missing-override
            -Wno-missing-braces
            -Wno-potentially-evaluated-expression
            -Wno-null-dereference
            -Wno-instantiation-after-specialization
            -Wno-user-defined-warnings
            -Wno-undefined-var-template
            -Wno-unused-private-field
    )
endif ()


# other flags
add_compile_options(
        -fno-math-errno
        -fno-trapping-math
        -fno-signed-zeros
        -msse2
        -march=x86-64
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fno-limit-debug-info)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(
            -fvisibility-inlines-hidden
            -ffloat-store
            -mfpmath=sse
            -fcx-limited-range
    )
endif ()

### LINKER OPTIONS ###

# use gold linker if installed
find_program(GOLD_LINKER ld.gold)
if (GOLD_LINKER)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
endif ()

### LIBRARIES ###

add_subdirectory(libraries)

### LINKING ###

# link_libraries(
# )

add_library(${PROJECT_NAME} ${SOURCE_FILES})

add_executable(${PROJECT_NAME}_test EXCLUDE_FROM_ALL ${TEST_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} UnitTest++)
target_include_directories(${PROJECT_NAME}_test PRIVATE tests)
