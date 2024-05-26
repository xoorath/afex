project(HelloWorld CXX)

declare_game_project(Core)

target_link_libraries(${PROJECT_NAME} 
        PRIVATE
            Core)