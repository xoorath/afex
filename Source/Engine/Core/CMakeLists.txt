project(Core CXX)

declare_engine_project()

find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        spdlog::spdlog
        glfw
)
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        "SPDLOG_ACTIVE_LEVEL=0"
)