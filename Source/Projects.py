import sys
sys.path.append("..")
from DevEnvironment.ProjectDefinition import *



# This method is where we define AFEX projects and their dependencies
# It is used by the build and package system and can also be imported directly by our 
# conanfiles to understand our targets and their dependencies
def GetAllProjects() -> list[ProjectBase]:
    return [
        ############################################################################### Engine components
        EngineProject(
            name="Core",
            directory="Engine/Core",
            conan_dependencies=[
                ConanDependency(
                    require="spdlog/1.14.1",
                    cmake_name = "spdlog::spdlog",
                    options=[ConanDependencyOption("shared", True)]
                )
            ],
            defines=["SPDLOG_ACTIVE_LEVEL=0"]
        ),

        EngineProject(
            name="Graphics",
            directory="Engine/Graphics",
            engine_dependencies=[EngineDependency(name="Core")],
            conan_dependencies=[
                # bgfx does not support static linkage. The graphics library will be responsible for
                # linking bgfx in and exporting functionality required by other modules.
                ConanDependency(
                        require="bgfx/cci.20230216",
                        cmake_name = "bgfx::bgfx"
                ),
                ConanDependency(
                    require="bimg/cci.20230114",
                    cmake_name = "bimg::bimg"
                )
            ]
        ),

        EngineProject(
            name="Platform", 
            directory="Engine/Platform",
            engine_dependencies=[
                EngineDependency(name="Core"),
                EngineDependency(name="Graphics")
            ],
            conan_dependencies=[
                ConanDependency(
                    require="glfw/3.4",
                    cmake_find = "glfw3",
                    options=[ConanDependencyOption("shared", True)]
                )
            ]
        ),

        ############################################################################### Games
        GameProject(
            name="HelloWorld",
            directory="HelloWorld",
            engine_dependencies=[
                EngineDependency(name="Core"),
                EngineDependency(name="Platform"),
                EngineDependency(name="Graphics")],
            startup=True
        ),
        
        GameProject(
            name="TowerDefense",
            directory="TowerDefense",
            engine_dependencies=[
                EngineDependency(name="Core"),
                EngineDependency(name="Platform"),
                EngineDependency(name="Graphics")]
        ),
    ]