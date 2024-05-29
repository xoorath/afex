from conan import ConanFile
from Engine.conanfile import requirements as EngineRequirements
from Engine.conanfile import configure as EngineConfigure
from HelloWorld.conanfile import requirements as HelloWorldRequirements
from TowerDefense.conanfile import requirements as TowerDefenseRequirements
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class sourceRecipe(ConanFile):
    name = "source"
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        EngineRequirements(self)
        HelloWorldRequirements(self)
        TowerDefenseRequirements(self)

    def configure(self):
        EngineConfigure(self)
        self.settings.compiler.cppstd="20"

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
