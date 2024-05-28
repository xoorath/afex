[Home](../README.md) > Developer Workflow

# Developer Workflow

AFEX is developed in a way that is intended to be friendly to platform ports and different development environments.
That said: only a narrow subset of tools are tested to work. Varying at all will require some work and know-how.

# Supported Development Environments

## Windows

1. Run `setup.py` from the project root. This will:
    * Automatically pull and build project dependencies from conan. (^1)
    * Generate project files and solutions.
    * Create a link to the solution files in the project root.
2. Open `source_debug.sln` from the project root. (^2)
3. Build and run Hello World.

At this point you are ready to work with AFEX and iterate.

To create a package:

1. In Visual Studio build the `3 - CMake/INSTALL` project.
2. Find the package output in `Generated/CMake_<config>/Package/<project name>`. (^3)

Packages can be cleaned and re-built from Visual Studio.

### Package fixup

At the time of writing: required dlls are not automatically copied to the game project package output. To fix the package output:

1. Copy the third-party dlls from `Generated/CMake_<config>/full_deploy/host/` next to the game executable in `Generated/CMake_<config>/Package/<project name>`
2. Copy first party dlls from `Generated/CMake_<config>/Package/<other project names>` next to the game executable in `Generated/CMake_<config>/Package/<project name>`

# Notes

* (^1): An internet connection is required the first time you pull project dependencies and any time you run setup after new dependencies have been added.
* (^2): Due to limitations in the current version of conan: we are unable to support multiple configurations in one Visual Studio solution (ie: release and debug). As a workaround all generated files are broken into config specific folders inside of `Generated/` and a separate Visual Studio solution is created for each.