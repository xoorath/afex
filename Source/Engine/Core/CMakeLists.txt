project(Core CXX)

declare_engine_project()

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        spdlog::spdlog
)
