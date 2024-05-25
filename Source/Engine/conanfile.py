from conan import ConanFile
from Engine.Core.conanfile import requirements as CoreRequirements
from Engine.Core.conanfile import configure as CoreConfigure

def requirements(self: ConanFile):
    CoreRequirements(self)


def configure(self: ConanFile):
    CoreConfigure(self)
