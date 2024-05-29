# Base class of all project definitions
class ConanDependencyOption:
    def __init__(self, name:str, value:any):
        self.name = name
        self.value = value

class ConanDependency:
    def __init__(self, *,
                    require :str, # Requires string. i.e: "spdlog/1.14.1"
                    cmake_find: str = None, # The string used in cmake with find_package. If None the require name will be used.
                    cmake_name: str = None, # The string used in cmake when linking this dependency. If None the require name will be used.
                    options: list[ConanDependencyOption] = None, # Options for this dependency.
                    is_public: bool = True # Is this a public or private dependency
                 ):
        self.require = require
        self.name = self.require.split('/')[0]
        self.cmake_find = self.name if cmake_find == None else cmake_find
        self.cmake_name = self.name if cmake_name == None else cmake_name
        self.options = options
        self.is_public = is_public
        ...

class EngineDependency:
    def __init__(self,
                    name: str,
                    is_public: bool = True # Is this a public or private dependency
                 ):
        self.name = name
        self.is_public = is_public


class ProjectBase:
    def __init__(self, *,
                 name :str,
                 directory :str,
                 conan_dependencies: list[ConanDependency] = None,
                 engine_dependencies: list[EngineDependency] = None,
                 defines :list[str] = None):
        self.name = name
        self.directory = directory
        self.conan_dependencies = conan_dependencies
        self.engine_dependencies = engine_dependencies
        self.defines = defines

class EngineProject(ProjectBase):
    def __init__(self, *,
                 name :str,
                 directory :str,
                 conan_dependencies: list[ConanDependency] = None,
                 engine_dependencies: list[EngineDependency] = None,
                 defines :list[str] = None):
        super(EngineProject, self).__init__(
            name=name,
            directory=directory,
            conan_dependencies=conan_dependencies,
            engine_dependencies=engine_dependencies,
            defines=defines)

class GameProject(ProjectBase):
    def __init__(self, *,
                 name :str,
                 directory :str,
                 conan_dependencies: list[ConanDependency] = None,
                 engine_dependencies: list[EngineDependency] = None,
                 defines :list[str] = None):
        super(GameProject, self).__init__(
            name=name,
            directory=directory,
            conan_dependencies=conan_dependencies,
            engine_dependencies=engine_dependencies,
            defines = defines)