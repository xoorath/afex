
function(declare_game_project engine_library_dependencies)
    declare_common_project(FALSE)
    
    if(WIN32)
        foreach(dep IN ITEMS ${engine_library_dependencies})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                "${PROJECT_BINARY_DIR}/../Engine/${dep}/$(configuration)/${dep}.dll"
                "$(TargetDir)"
            )
        endforeach()

        set(CONAN_DEPLOYER_DIR "${CMAKE_BINARY_DIR}/full_deploy/host")

        if(CMAKE_CONFIGURATION_TYPES)
            file(GLOB_RECURSE dll_files_to_copy "${CMAKE_BINARY_DIR}/bin/${config}/*/*.dll")
        else()
            file(GLOB_RECURSE dll_files_to_copy "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/*/*.dll")
        endif()

        foreach(dll IN ITEMS ${dll_files_to_copy})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                "${dll}"
                "$(TargetDir)"
            )
        endforeach()
    endif()

    copy_third_party_dlls()
    
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "1 Game")
endfunction(declare_game_project)