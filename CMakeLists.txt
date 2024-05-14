cmake_minimum_required(VERSION 3.15)
project(afex CXX)



add_executable(afex src/afex.cpp src/main.cpp)



install(TARGETS afex DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )
