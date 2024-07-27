[Home](../README.md) > Setting up a development environment

# Setting up a development environment

AFEX is developed in a way that is intended to be friendly to platform ports and different development environments.
That said: only a narrow subset of tools are tested to work. Varying at all will require some work and know-how.

# Supported Development Environments

## Windows

The following tools are used for the windows development environment and should be installed before the development environment can be contributed to.

* [Visual Studio 2022 (Version 17.10.0)](https://visualstudio.microsoft.com/downloads/)
* [CMake 3.29](https://cmake.org/download/)
* [Conan 2.4.1conan](https://conan.io/downloads)
* [Python 3.11.5](https://www.python.org/downloads/)

For CMake, Conan and Python: please check that these tools are available at your system path by running their help commands from a terminal.

Next: [Developer Workflow](./DevWorkflow.md)