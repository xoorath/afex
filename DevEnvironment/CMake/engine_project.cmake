
# Helper to setup the common structure of engine libraries.
function(declare_engine_project)
    declare_common_project(TRUE)
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "0 Engine")
endfunction(declare_engine_project)