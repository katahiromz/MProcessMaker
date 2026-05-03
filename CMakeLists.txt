# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.10)

# project name and languages
project(MProcessMaker CXX RC)

# We don't like run-time DLLs
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    set(CMAKE_CXX_FLAGS "-static")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    set(CMAKE_CXX_FLAGS "-static")
elseif (MSVC)
    # replace "/MD" with "/MT" (building without runtime DLLs)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    foreach(CompilerFlags ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlags} "${${CompilerFlags}}")
    endforeach()
endif()

# MProcessMaker.exe
add_executable(MProcessMaker WIN32 main.cpp main_res.rc)
target_compile_definitions(MProcessMaker PRIVATE UNICODE _UNICODE)
target_link_libraries(MProcessMaker comctl32)
if(MSVC)
    set_target_properties(MProcessMaker PROPERTIES LINK_FLAGS "/MANIFEST:NO")
endif()

##############################################################################
