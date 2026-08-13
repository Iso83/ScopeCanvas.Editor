cmake_minimum_required(VERSION 4.0)

project(SC_Editor
    VERSION 0.1.0
    LANGUAGES C CXX
)

# =========================================================
# Project configuration
# =========================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# =========================================================
# CppCMake
# =========================================================

include(cmake/Dependencies.cmake)

dependencies_fetch_cppcmake()

cppcmake_project_initialize()

if(CPPCMAKE_PROJECT_ALREADY_REGISTERED)
    return()
endif()

cppcmake_githook_precommit()
cppcmake_version_generate()
cppcmake_test_setup()

# =========================================================
# Dependencies
# =========================================================

dependencies_setup()

if(NOT TARGET ScopeCanvas_engine_core)
    cppcmake_dependency_add_subdirectory("extern/ScopeCanvas")
endif()

# =========================================================
# Modules
# =========================================================

cppcmake_module_add(editor/text)