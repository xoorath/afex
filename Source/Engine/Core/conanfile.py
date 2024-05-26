from conan import ConanFile

def requirements(self: ConanFile):
    self.requires("spdlog/1.14.1")
    self.requires("glfw/3.4")
    ...

def configure(self: ConanFile):
     self.options["spdlog"].shared = True
     self.options["glfw"].shared = True