file(STRINGS "$ENV{HOME}/.emscripten" EMSCRIPTEN_DOT_CONTENT)
foreach(LINE ${EMSCRIPTEN_DOT_CONTENT})
string(REPLACE "=" ";" VAR_VALUE ${LINE})
list(LENGTH VAR_VALUE VAR_VALUE_LENGTH)
if (VAR_VALUE_LENGTH EQUAL 2)
list(GET VAR_VALUE 0 VAR)
list(GET VAR_VALUE 1 VALUE)
if (VAR STREQUAL "EMSCRIPTEN_ROOT")
STRING(REGEX REPLACE "'" "" EMSCRIPTEN_ROOT_PATH ${VALUE})
endif()
endif()
endforeach()

set(EMSCRIPTEN 1)

# enable the emscripten tracer if profiling is enabled
if (PROFILING)
    set(EMSCRIPTEN_TRACING 1)
    set(EMSCRIPTEN_TRACING_OPTION "--tracing")
else()
    set(EMSCRIPTEN_TRACING 0)
    set(EMSCRIPTEN_TRACING_OPTION "")
endif()

# total memory is 128MB for main thread, and 16 MB for worker
# NOTE: USE_MEMORY_INIT_FILE has/had problems that the script is already starting but the MEM file isn't loaded yet(?)
# at least I'm having weird startup problems...
set(EMSCRIPTEN_TOTAL_MEMORY 134217728)
set(EMSCRIPTEN_TOTAL_MEMORY_WORKER 16777216)
set(EMSCRIPTEN_USE_MEMORY_INIT_FILE 1)
set(EMSCRIPTEN_OPT "-O3")
set(EMSCRIPTEN_LTO_LEVEL 1)
set(EMSCRIPTEN_NO_FILESYSTEM 0)
#set(EMSCRIPTEN_NO_FILESYSTEM 1)

# disable closure for now, as long as ANGLE_instanced_array support is not fully supported in emscripten
set(EMSCRIPTEN_USE_CLOSURE 0)
set(EMSCRIPTEN_ASSERTIONS 0)
set(EMSCRIPTEN_OUTLINING_LIMIT 20000)

set(EMSCRIPTEN_BUILD_VERBOSE 0)
set(EMSCRIPTEN_DISABLE_EXCEPTION_CATCHING 1)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(COMPILING on)
set(CMAKE_CROSSCOMPILING TRUE)
set(EMSCRIPTEN_CONFIG_OPTION "")
set(EMSCRIPTEN_CACHE_OPTION "")

# tool suffic (.bat on windows)
if (CMAKE_HOST_WIN32)
    set(EMCC_SUFFIX ".bat")
else()
    set(EMCC_SUFFIX "")
endif()

include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER("${CMAKE_C_COMPILER}" Clang)
CMAKE_FORCE_CXX_COMPILER("${CMAKE_CXX_COMPILER}" Clang)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

set(CMAKE_C_COMPILER "${EMSCRIPTEN_ROOT_PATH}/emcc${EMCC_SUFFIX}" CACHE PATH "gcc" FORCE)
set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_ROOT_PATH}/em++${EMCC_SUFFIX}" CACHE PATH "g++" FORCE)
set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar${EMCC_SUFFIX}" CACHE PATH "archive" FORCE)
set(CMAKE_LINKER "${EMSCRIPTEN_ROOT_PATH}/emcc${EMCC_SUFFIX}" CACHE PATH "linker" FORCE)
set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib${EMCC_SUFFIX}" CACHE PATH "ranlib" FORCE)

# only search for libraries and includes in the toolchain
set(CMAKE_FIND_ROOT_PATH ${EMSCRIPTEN_ROOT_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_SYSTEM_INCLUDE_PATH "${EMSCRIPTEN_ROOT_PATH}/system/include")

# c++ compiler flags
set(CMAKE_CXX_FLAGS "${EMSCRIPTEN_CONFIG_OPTION} ${EMSCRIPTEN_CACHE_OPTION} ${EMSCRIPTEN_TRACING_OPTION} -std=c++11 -stdlib=libc++ -fno-rtti -fno-exceptions -fstrict-aliasing -Wall -Wno-warn-absolute-paths -Wno-multichar -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -Wno-ignored-qualifiers -Wno-long-long -Wno-overloaded-virtual -Wno-deprecated-writable-strings -Wno-unused-volatile-lvalue -Wno-inconsistent-missing-override")
set(CMAKE_CXX_FLAGS_RELEASE "${EMSCRIPTEN_OPT} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${EMSCRIPTEN_OPT} -g -D_DEBUG_ -D_DEBUG -DDEBUG=1")
set(CMAKE_CXX_FLAGS_PROFILING "${EMSCRIPTEN_OPT} --profiling")

# c compiler flags
set(CMAKE_C_FLAGS "${EMSCRIPTEN_CONFIG_OPTION} ${EMSCRIPTEN_CACHE_OPTION} ${EMSCRIPTEN_TRACING_OPTION} -fstrict-aliasing -Wall -Wno-warn-absolute-paths -Wextra -Wno-multichar -Wno-unused-parameter -Wno-unknown-pragmas -Wno-ignored-qualifiers -Wno-long-long -Wno-overloaded-virtual -Wno-deprecated-writable-strings -Wno-unused-volatile-lvalue")
set(CMAKE_C_FLAGS_RELEASE "${EMSCRIPTEN_OPT} -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${EMSCRIPTEN_OPT} -g -D_DEBUG_ -D_DEBUG -DDEBUG=1")
set(CMAKE_C_FLAGS_PROFILING "${EMSCRIPTEN_OPT} --profiling")

# linker flags
set(CMAKE_EXE_LINKER_FLAGS "${EMSCRIPTEN_CONFIG_OPTION} ${EMSCRIPTEN_CACHE_OPTION} ${EMSCRIPTEN_TRACING_OPTION} --memory-init-file ${EMSCRIPTEN_USE_MEMORY_INIT_FILE} -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s TOTAL_MEMORY=${EMSCRIPTEN_TOTAL_MEMORY} -s DISABLE_EXCEPTION_CATCHING=${EMSCRIPTEN_DISABLE_EXCEPTION_CATCHING} -s NO_FILESYSTEM=${EMSCRIPTEN_NO_FILESYSTEM}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${EMSCRIPTEN_OPT} --llvm-lto ${EMSCRIPTEN_LTO_LEVEL} -s VERBOSE=${EMSCRIPTEN_BUILD_VERBOSE} -s ASM_JS=1 -s ASSERTIONS=${EMSCRIPTEN_ASSERTIONS} -s OUTLINING_LIMIT=${EMSCRIPTEN_OUTLINING_LIMIT} --closure ${EMSCRIPTEN_USE_CLOSURE}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${EMSCRIPTEN_OPT} -g -s ASM_JS=1 -s VERBOSE=${EMSCRIPTEN_BUILD_VERBOSE}")
set(CMAKE_EXE_LINKER_FLAGS_PROFILING "--profiling ${EMSCRIPTEN_OPT} --llvm-lto ${EMSCRIPTEN_LTO_LEVEL} -s VERBOSE=${EMSCRIPTEN_BUILD_VERBOSE} -s ASM_JS=1 -s ASSERTIONS=${EMSCRIPTEN_ASSERTIONS} -s OUTLINING_LIMIT=${EMSCRIPTEN_OUTLINING_LIMIT}")

# static library flags (for CMAKE_AR)
set(CMAKE_STATIC_LINKER_FLAGS "${EMSCRIPTEN_CONFIG_OPTION}")

# dynamic lib linker flags
set(CMAKE_SHARED_LINKER_FLAGS "-shared ${EMSCRIPTEN_CONFIG_OPTIONS} ${EMSCRIPTEN_CACHE_OPTION} ${EMSCRIPTEN_TRACING_OPTION} --memory-init-file 0 -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s TOTAL_MEMORY=${EMSCRIPTEN_TOTAL_MEMORY_WORKER} -s BUILD_AS_WORKER=1 -s EXPORTED_FUNCTIONS=[\\\"_dowork\\\"] -s DISABLE_EXCEPTION_CATCHING=${EMSCRIPTEN_DISABLE_EXCEPTION_CATCHING} -s NO_FILESYSTEM=${EMSCRIPTEN_NO_FILESYSTEM}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${EMSCRIPTEN_OPT} --llvm-lto ${EMSCRIPTEN_LTO_LEVEL} -s VERBOSE=${EMSCRIPTEN_BUILD_VERBOSE} -s ASM_JS=1 -s ASSERTIONS=${EMSCRIPTEN_ASSERTIONS} -s OUTLINING_LIMIT=${EMSCRIPTEN_OUTLINING_LIMIT} --closure ${EMSCRIPTEN_USE_CLOSURE}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${EMSCRIPTEN_OPT} -g -s ASM_JS=1 -s VERBOSE=${EMSCRIPTEN_BUILD_VERBOSE} --closure 0")
set(CMAKE_SHARED_LINKER_FLAGS_PROFILING "--profiling ${EMSCRIPTEN_OPT} --llvm-lto ${EMSCRIPTEN_LTO_LEVEL} -s VERBOSE=${EMSCRIPTEN_BUILD_VERBOSE} -s ASM_JS=1 -s ASSERTIONS=${EMSCRIPTEN_ASSERTIONS} -s OUTLINING_LIMIT=${EMSCRIPTEN_OUTLINING_LIMIT}")
