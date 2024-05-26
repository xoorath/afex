
# Helper to setup the common structure of engine libraries.
function(declare_engine_project)
    set(PRIVATE_DIR "Private")
    set(PUBLIC_DIR "Public")

    file(GLOB_RECURSE PRIVATE_FILES "${PRIVATE_DIR}/*")
    file(GLOB_RECURSE PUBLIC_FILES  "${PUBLIC_DIR}/*")

    add_library(${PROJECT_NAME}
        SHARED
            ${PRIVATE_FILES}
            ${PUBLIC_FILES}
            conanfile.py
    )

    install(TARGETS ${PROJECT_NAME} DESTINATION "."
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
    )

        
    generate_export_header(${PROJECT_NAME}
        EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/GeneratedHeaders/${PROJECT_NAME}/include/${PROJECT_NAME}/${PROJECT_NAME}.export.h"
    )

    target_include_directories(${PROJECT_NAME}
        PUBLIC
            "Public/"
            "${CMAKE_BINARY_DIR}/GeneratedHeaders/${PROJECT_NAME}/include/"
    )

    if(MSVC)
        foreach(_source IN ITEMS ${PRIVATE_FILES} ${PUBLIC_FILES})
            get_filename_component(_source_path "${_source}" PATH)
            string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _group_path "${_source_path}")
            string(REPLACE "/" "\\" _group_path "${_group_path}")
            source_group("${_group_path}" FILES "${_source}")
        endforeach()

        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "0 Engine")

        target_compile_definitions(${PROJECT_NAME}
            PUBLIC
                "_CRT_SECURE_NO_WARNINGS=1"
        )
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

endfunction(declare_engine_project)