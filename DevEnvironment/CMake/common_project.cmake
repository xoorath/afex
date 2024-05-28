function(declare_common_project is_library)

    set(PRIVATE_DIR "Private")
    set(PUBLIC_DIR "Public")

    file(GLOB_RECURSE PRIVATE_FILES "${PRIVATE_DIR}/*")
    file(GLOB_RECURSE PUBLIC_FILES  "${PUBLIC_DIR}/*")

    if (${is_library})
        add_library(${PROJECT_NAME} SHARED ${PRIVATE_FILES} ${PUBLIC_FILES} conanfile.py)
    else()
        add_executable(${PROJECT_NAME} ${PRIVATE_FILES} ${PUBLIC_FILES} conanfile.py)
    endif()

    if(MSVC)
        foreach(_source IN ITEMS ${PRIVATE_FILES} ${PUBLIC_FILES})
            get_filename_component(_source_path "${_source}" PATH)
            string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _group_path "${_source_path}")
            string(REPLACE "/" "\\" _group_path "${_group_path}")
            source_group("${_group_path}" FILES "${_source}")
        endforeach()

        target_compile_definitions(${PROJECT_NAME}
            PUBLIC
                "_CRT_SECURE_NO_WARNINGS=1"
        )
        target_compile_options(${PROJECT_NAME} 
            PRIVATE 
                /GR- # Disable RTTI
        )        
        target_compile_options(${PROJECT_NAME} 
            PRIVATE 
                /W4 /WX # Set warning level
        )
    else()
        target_compile_options(${PROJECT_NAME} 
            PRIVATE 
                -fno-rtti # Disable RTTI
        )
        target_compile_options(${TARGET_NAME} 
            PRIVATE
                -Wall -Wextra -Wpedantic -Werror # Set warning level
        )
    endif()
endfunction(declare_common_project)