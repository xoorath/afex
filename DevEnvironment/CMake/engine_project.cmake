
# Helper to setup the common structure of engine libraries.
function(declare_engine_project)
    declare_common_project(TRUE)
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "0 Engine")
    generate_export_header(${PROJECT_NAME}
            EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/GeneratedHeaders/${PROJECT_NAME}/include/${PROJECT_NAME}/${PROJECT_NAME}.export.h"
    )
    target_include_directories(${PROJECT_NAME}
        PUBLIC
            "Public/"
            "${CMAKE_BINARY_DIR}/GeneratedHeaders/${PROJECT_NAME}/include/"
    )
endfunction(declare_engine_project)