from conan import ConanFile
from Engine.Core.conanfile import requirements as CoreRequirements
from Engine.Core.conanfile import configure as CoreConfigure
from Engine.Platform.conanfile import requirements as PlatformRequirements
from Engine.Platform.conanfile import configure as PlatformConfigure

def requirements(self: ConanFile):
    CoreRequirements(self)
    PlatformRequirements(self)

def configure(self: ConanFile):
    CoreConfigure(self)
    PlatformConfigure(self)
