include_guard(GLOBAL)

include(FetchContent)

function(dependencies_fetch_cppcmake)
    # =========================================================
    # Summary
    #
    # Makes the CppCMake API available by:
    #   - Returning immediately when CppCMake is already loaded.
    #   - Fetching the CppCMake repository when required.
    #   - Loading the CppCMake API.
    # =========================================================

    if(COMMAND cppcmake_project_initialize)
        return()
    endif()

    FetchContent_Declare(
        CppCMake
        GIT_REPOSITORY https://github.com/Iso83/CppCMake.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(CppCMake)
	
    include("${cppcmake_SOURCE_DIR}/cmake/CppCMake.cmake")
endfunction()


macro(dependencies_setup)
    # =========================================================
    # Summary
    #
    # Configures all third-party dependencies required by
    # SC_Editor.
    #
    # The resolved targets are returned through local variables
    # for use by the project's CMake configuration.
    # =========================================================
    
    if(NOT TARGET ScopeCanvas_engine_core)
        cppcmake_gitsubmodule_init(
            QUIET 
            WORKING_DIRECTORY 
                "${CMAKE_CURRENT_SOURCE_DIR}" 
            PATH 
                "extern/ScopeCanvas"
        )
    endif()

endmacro()