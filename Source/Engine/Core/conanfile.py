from conan import ConanFile

def requirements(self: ConanFile):
    self.requires("spdlog/1.14.1")
    ...

def configure(self: ConanFile):
     self.options["spdlog"].shared = True