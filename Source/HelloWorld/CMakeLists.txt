# GENERATED FILE: See Setup.py
project(HelloWorld CXX)
declare_game_project("Core;Platform")
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Core
        Platform
)
