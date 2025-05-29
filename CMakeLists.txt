cmake_minimum_required(VERSION 3.12)

project(kiwi-grammar VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include_directories(include/)
include_directories(Kiwi/include)
include_directories(Kiwi/third_party/json/include)

set(KIWI_USE_MIMALLOC ON CACHE BOOL "Use mimalloc allocator")
if (EMSCRIPTEN)
    set(KIWI_USE_CPUINFO OFF CACHE BOOL "Use cpuinfo library")
else()
    set(KIWI_USE_CPUINFO ON CACHE BOOL "Use cpuinfo library")
endif()
set(KIWI_BUILD_CLI OFF CACHE BOOL "Build CLI")
set(KIWI_BUILD_EVALUATOR OFF CACHE BOOL "Build evaluator")
set(KIWI_BUILD_MODEL_BUILDER OFF CACHE BOOL "Build model builder")
set(KIWI_BUILD_TEST OFF CACHE BOOL "Build tests")
set(KIWI_JAVA_BINDING OFF CACHE BOOL "Build Java binding")

add_subdirectory(Kiwi)

set(COMMON_SRCS
    src/rules.cpp
    src/ruleset.cpp
)

if (EMSCRIPTEN) 
    add_executable(kiwi-grammar ${COMMON_SRCS} src/main-wasm.cpp)
    target_link_libraries(kiwi-grammar PRIVATE kiwi_static)
    
    set_target_properties(kiwi-grammar PROPERTIES
        LINK_FLAGS "--bind -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s DYNAMIC_EXECUTION=0 -s EXPORT_ES6=1 -s ENVIRONMENT=worker -s USE_ES6_IMPORT_META=0 -s MODULARIZE=1 -s EXPORT_NAME=grammar -s 'EXPORTED_RUNTIME_METHODS=[\"FS\"]'"
    )
else()
    add_executable(kiwi-grammar ${COMMON_SRCS} src/main.cpp)
    target_link_libraries(kiwi-grammar PRIVATE kiwi_static)
endif()
