from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class afexRecipe(ConanFile):
    name = "afex"
    version = "1.0"
    package_type = "application"

    # Optional metadata
    license = "CC BY-NC 4.0"
    author = "Jared Thomson <jared.thomson@xoorath.com>"
    url = "https://github.com/xoorath/afex"
    description = "Another fucking engine by xoorath."
    topics = ("Game Engine", "Game Development")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

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

    

    
