cmake_minimum_required(VERSION 3.20)

project(gfxp VERSION 0.1.0 LANGUAGES CXX)

option(GFXP_STRICT "Enable strict compiler warnings" ON)
option(GFXP_WARN_AS_ERROR "Treat warnings as errors" OFF)
option(GFXP_BUILD_EXAMPLES "Build example programs" ON)
option(GFXP_BUILD_TESTS "Build test programs" ON)
option(GFXP_BUILD_BENCHMARKS "Build benchmark programs" ON)

add_library(gfxp INTERFACE)
add_library(gfxp::gfxp ALIAS gfxp)

target_include_directories(gfxp
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(gfxp INTERFACE cxx_std_20)

function(gfxp_apply_warnings target_name)
    if(NOT GFXP_STRICT)
        return()
    endif()

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
        if(GFXP_WARN_AS_ERROR)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
        )
        if(GFXP_WARN_AS_ERROR)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()
endfunction()

if(GFXP_BUILD_EXAMPLES)
    add_executable(gfxp_demo examples/gfxp_demo.cpp)
    target_link_libraries(gfxp_demo PRIVATE gfxp::gfxp)
    target_compile_features(gfxp_demo PRIVATE cxx_std_20)
    gfxp_apply_warnings(gfxp_demo)
endif()

if(GFXP_BUILD_TESTS)
    enable_testing()

    add_executable(gfxp_core_tests tests/core_tests.cpp)
    target_link_libraries(gfxp_core_tests PRIVATE gfxp::gfxp)
    target_compile_features(gfxp_core_tests PRIVATE cxx_std_20)
    gfxp_apply_warnings(gfxp_core_tests)

    add_test(NAME gfxp_core_tests COMMAND gfxp_core_tests)
endif()

if(GFXP_BUILD_BENCHMARKS)
    add_executable(gfxp_bench bench/gfxp_bench.cpp)
    target_link_libraries(gfxp_bench PRIVATE gfxp::gfxp)
    target_compile_features(gfxp_bench PRIVATE cxx_std_20)
    gfxp_apply_warnings(gfxp_bench)
endif()
