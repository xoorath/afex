from conan import ConanFile

def requirements(self: ConanFile):
    self.requires("glfw/3.4")
    ...

def configure(self: ConanFile):
     self.options["glfw"].shared = True