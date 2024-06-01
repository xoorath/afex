
function(declare_game_project engine_library_dependencies)
    declare_common_project(FALSE)

    target_include_directories(${PROJECT_NAME}
        PUBLIC
            "Public/"
    )

    ########################################################## Collect info on dependencies

    foreach(dep IN ITEMS ${engine_library_dependencies})
        list(APPEND DEBUGGER_ENVIRONMENT "${CMAKE_BINARY_DIR}/Engine/${dep}/$(Configuration)/")
    endforeach()

    set(CONAN_DEPLOYER_DIR "${CMAKE_BINARY_DIR}/full_deploy/host")
    file(GLOB_RECURSE dll_files_to_copy "${CONAN_DEPLOYER_DIR}/*/*.dll")
    foreach(dll IN ITEMS ${dll_files_to_copy})
        get_filename_component(dllDirectory ${dll} DIRECTORY)
        list(APPEND DEBUGGER_ENVIRONMENT ${dllDirectory})
    endforeach()
    
    ########################################################## Setup Visual Studio

    if(MSVC)
        SET(USER_FILE_TEMPLATE "${CMAKE_BINARY_DIR}/../../DevEnvironment/CMake/GameProjectUserTemplate.vcxproj.user")
        SET(USER_FILE_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user")
        list(REMOVE_DUPLICATES DEBUGGER_ENVIRONMENT)
        CONFIGURE_FILE(${USER_FILE_TEMPLATE} ${USER_FILE_DESTINATION} @ONLY)

        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "1 Game")
    endif()

    copy_third_party_dlls()
    
endfunction(declare_game_project)