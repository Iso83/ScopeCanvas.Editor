# ============================================================================
# Source files
# ============================================================================

file(GLOB_RECURSE MODULE_SRC CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE MODULE_PRIVATE_HEADERS CONFIGURE_DEPENDS
    src/*.h
)

file(GLOB_RECURSE MODULE_PUBLIC_HEADERS CONFIGURE_DEPENDS
    include/*.h
)

# ============================================================================
# Target
# ============================================================================

string(REPLACE "/" "_" MODULE_NAME "editor/text")

set(MODULE_TARGET "SC_${MODULE_NAME}")

add_library(${MODULE_TARGET}
    ${MODULE_SRC}
    ${MODULE_PRIVATE_HEADERS}
    ${MODULE_PUBLIC_HEADERS}
)

add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_TARGET})

# ============================================================================
# Build configuration
# ============================================================================

cppcmake_version_target(${MODULE_TARGET})
cppcmake_helper_include_target(${MODULE_TARGET})

target_include_directories(${MODULE_TARGET}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(${MODULE_TARGET}
    PUBLIC
        cxx_std_20
)

# ============================================================================
# IDE organization
# ============================================================================

cppcmake_project_set_ide_folder(${MODULE_TARGET} "editor/text")

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/ScopeCanvas/editor/text"
    PREFIX "Include"
    FILES
        ${MODULE_PUBLIC_HEADERS}
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source"
    FILES
        ${MODULE_SRC}
        ${MODULE_PRIVATE_HEADERS}
)

# ============================================================================
# Dependencies
# ============================================================================

target_link_libraries(${MODULE_TARGET}
    PUBLIC
        ScopeCanvas_engine_render
        ${FREETYPE}
)

if(NOT EMSCRIPTEN)
    find_package(Threads REQUIRED)
    target_link_libraries(${MODULE_TARGET} 
        PRIVATE 
            Threads::Threads
    )
endif()

# ============================================================================
# Tests
# ============================================================================

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_annotationModel
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/annotation"
    SOURCES
        tests/annotation/Model.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Layout
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/diff"
    SOURCES
        tests/diff/Layout.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_inputController
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/input"
    SOURCES
        tests/input/Controller.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_DrawContext
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/render/window"
    SOURCES
        tests/render/window/DrawContext.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_rWndModel
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/render/window"
    SOURCES
        tests/render/window/Model.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Vertical
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/scroll"
    SOURCES
        tests/scroll/Vertical.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_EditorSession
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/session"
    SOURCES
        tests/session/EditorSession.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_profileCPP
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/syntax/profiles"
    SOURCES
        tests/syntax/profiles/Cpp.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_AnalysisScheduler
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/syntax"
    SOURCES
        tests/syntax/AnalysisScheduler.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_syntaxDocument
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/syntax"
    SOURCES
        tests/syntax/Document.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Highlight
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/syntax"
    SOURCES
        tests/syntax/Highlighter.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_zoomController
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text/zoom"
    SOURCES
        tests/zoom/Controller.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_textDocument
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text"
    SOURCES
        tests/Document.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_RendererCache
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text"
    SOURCES
        tests/RendererCache.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Search
    TARGET
        ${MODULE_TARGET}
    MODULE
        "editor/text"
    SOURCES
        tests/Search.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)