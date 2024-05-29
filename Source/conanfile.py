from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from Projects import GetAllProjects

ALL_PROJECTS = GetAllProjects()

class sourceRecipe(ConanFile):
    name = "source"
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        allRequires = []
        for project in ALL_PROJECTS:
            if project.conan_dependencies:
                for dep in project.conan_dependencies:
                    if dep.require not in allRequires: 
                        allRequires.append(dep.require)
        for req in allRequires:
            self.requires(req)
            

    def configure(self):
        self.settings.compiler.cppstd="20"
        for project in ALL_PROJECTS:
            if project.conan_dependencies:
                for dep in project.conan_dependencies:
                    for option in dep.options:
                        self.options[dep.name][option.name] = option.value

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
