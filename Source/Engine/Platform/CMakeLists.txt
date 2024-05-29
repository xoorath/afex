# GENERATED FILE: See Setup.py
project(Platform CXX)
declare_engine_project()
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Core
        glfw
)
