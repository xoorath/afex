[Home](../README.md) > Developer Workflow

# Developer Workflow

AFEX is developed in a way that is intended to be friendly to platform ports and different development environments.
That said: only a narrow subset of tools are tested to work. Varying at all will require some work and know-how.

# Supported Development Environments

## Windows

### Build and debug:

1. Run `Setup.py` from the project root. This will:
    * Automatically pull and build project dependencies from conan. (^1)
    * Generate project files and solutions.
    * Create a link to the solution files in the project root.
2. Open `source_debug.sln` from the project root. (^2)
3. Build and run Hello World or other projects from the Game folder in the solution.

At this point you are ready to work with AFEX and iterate.

### Create a package:

1. Build the projects you want to package. (^3)
2. Run `Package.py` from the project root. (^4)
3. Find the package output in `Generated/Packages/`

The layout of a package follows this format: 

```
Generated/
Generated/<ProjectName>/
Generated/<ProjectName>/<Config>/
Generated/<ProjectName>/<Config>/<ProjectName>.pdb
Generated/<ProjectName>/<Config>/<ProjectName>/
Generated/<ProjectName>/<Config>/<ProjectName>/<PackageContents>
```

The directory named `<PackageContents>` above is the directory you would ship to customers. 
If everything packaged successfully running the executable in that directory should be very-nearly equivalent to running that project in a debugger.

One directory up from that are the pdb files needed to debug that executable. You should always retain the debug symbols for builds you ship to customers. 
The best practice when creating a shipping build is to build debug and release flavors and to archive the entire `Generated/<ProjectName>/` directory in addition to the `Generated/<ProjectName>/Release/<ProjectName>/<PackageContents>` directory that will be zipped and shipped.

# Notes

* (^1): An internet connection is required the first time you pull project dependencies and any time you run setup after new dependencies have been added.
* (^2): Due to limitations in the current version of conan: we are unable to support multiple configurations in one Visual Studio solution (ie: release and debug). As a workaround all generated files are broken into config specific folders inside of `Generated/` and a separate Visual Studio solution is created for each.
* (^3): Opening the debug and release solutions and building the entire solution is sufficient. That said: you only have to build the targets you intend to package.
* (^4): You can run `Package.py --help` for more information about packaging args and how to select specific projects or configs. 