import sys
sys.path.append("..")
from DevEnvironment.ProjectDefinition import *

DearImGui=ConanDependency(require="imgui/1.90.7", cmake_name = "imgui::imgui", options=[ConanDependencyOption("shared", True)])

# This method is where we define AFEX projects and their dependencies
# It is used by the build and package system and can also be imported directly by our 
# conanfiles to understand our targets and their dependencies
def GetAllProjects() -> list[ProjectBase]:
    global DearImGui
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
                ),
                ConanDependency(
                    require="tomlplusplus/3.4.0",
                    cmake_name = "tomlplusplus::tomlplusplus",
                    options=[
                        ConanDependencyOption("shared", True),
                        ConanDependencyOption("TOML_EXCEPTIONS", False)
                    ]
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
                ),
                DearImGui
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
                ),
                DearImGui
            ]
        ),

        EngineProject(
            name="Application", 
            directory="Engine/Application",
            engine_dependencies=[
                EngineDependency(name="Core"),
                EngineDependency(name="Platform"),
                EngineDependency(name="Graphics")
            ]
        ),

        ############################################################################### Games
        GameProject(
            name="HelloWorld",
            directory="HelloWorld",
            engine_dependencies=[
                EngineDependency(name="Core"),
                EngineDependency(name="Platform"),
                EngineDependency(name="Graphics"),
                EngineDependency(name="Application")
            ]
        ),
        
        GameProject(
            name="Donsol",
            directory="Donsol",
            engine_dependencies=[
                EngineDependency(name="Core"),
                EngineDependency(name="Platform"),
                EngineDependency(name="Graphics"),
                EngineDependency(name="Application")
            ],
            startup=True
        ),
    ]