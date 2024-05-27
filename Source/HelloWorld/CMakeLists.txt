project(HelloWorld CXX)

declare_game_project("Core;Platform")

target_link_libraries(${PROJECT_NAME} 
        PRIVATE
            Core
            Platform)