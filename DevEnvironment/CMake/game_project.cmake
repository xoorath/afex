
function(declare_game_project engine_library_dependencies)
    set(PRIVATE_DIR "Private")
    set(PUBLIC_DIR "Public")

    file(GLOB_RECURSE PRIVATE_FILES "${PRIVATE_DIR}/*")
    file(GLOB_RECURSE PUBLIC_FILES  "${PUBLIC_DIR}/*")

    add_executable(${PROJECT_NAME} ${PRIVATE_FILES} ${PUBLIC_FILES} conanfile.py)

    install(TARGETS ${PROJECT_NAME} DESTINATION "."
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
    )
    
    if(WIN32)
        set(CONAN_DEPLOYER_DIR "${CMAKE_BINARY_DIR}/full_deploy/host")

        foreach(dep IN ITEMS ${engine_library_dependencies})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                "${PROJECT_BINARY_DIR}/../Engine/${dep}/$(configuration)/${dep}.dll"
                "$(TargetDir)"
            )
        endforeach()

        foreach(_source IN ITEMS ${PRIVATE_FILES} ${PUBLIC_FILES})
            get_filename_component(_source_path "${_source}" PATH)
            string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _group_path "${_source_path}")
            string(REPLACE "/" "\\" _group_path "${_group_path}")
            source_group("${_group_path}" FILES "${_source}")
        endforeach()

        copy_third_party_dlls()

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

        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "1 Game")
        # Disable RTTI
        target_compile_options(${PROJECT_NAME} PRIVATE /GR-)
        # Set warning level
        target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
    else()
        # Disable RTTI
        target_compile_options(${PROJECT_NAME} PRIVATE -fno-rtti)
        # Set warning level
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction(declare_game_project)