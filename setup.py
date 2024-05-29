import os
from Source.Projects import *
from DevEnvironment.ProjectDefinition import EngineProject, GameProject
from DevEnvironment.BuildFormatting import BeginSection, EndSection, PrintContext, PrintError

TEMP_DIR_NAME = "Generated"
ALL_PROJECTS = GetAllProjects()
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TEMP_DIR = os.path.join(SCRIPT_DIR, TEMP_DIR_NAME)
# Build config support is an involved topic - you need to support configs in CMake
# and map them to optimization levels in Conan.
BUILD_CONFIGS = ["Debug", "Release"]

def GetConanTempDir(buildType : str):
    return os.path.join(TEMP_DIR, f"Conan_{buildType}")

def GetCMakeTempDir(buildType : str):
    return os.path.join(TEMP_DIR, f"CMake_{buildType}")

def SetupDirectories():
    os.makedirs(TEMP_DIR, exist_ok=True)
    for buildType in BUILD_CONFIGS:
        os.makedirs(GetConanTempDir(buildType), exist_ok=True)
        os.makedirs(GetCMakeTempDir(buildType), exist_ok=True)

def CheckForTools():
    from shutil import which
    for program in ["conan", "cmake"]:
        if which(program) is not None:
            PrintContext(f"{program}: found")
        else:
            PrintError(f"{program} not found", doExcept=True)

def SetupConan(buildType : str):
    import subprocess
    import glob

    for project in ALL_PROJECTS:
        conanTempDir = GetConanTempDir(buildType)
        presetsFiles = glob.glob(os.path.join(SCRIPT_DIR, "Source", project.directory, "**" ,"CMakePresets.json"))
        for presetsFile in presetsFiles:
            if os.path.exists(presetsFile) and os.path.isfile(presetsFile):
                os.remove(presetsFile)

        conanfile = os.path.join(SCRIPT_DIR, "Source", "conanfile.py")
        completedProcess = subprocess.run(["conan",
                        "install", conanfile,
                        "-of", conanTempDir,
                        "-s", f"build_type={buildType}",
                        "--build=missing",
                        "--deployer=full_deploy", 
                        f"--deployer-folder={GetCMakeTempDir(buildType)}"], 
                        check=True) 
        if completedProcess.returncode != 0:
            PrintError(f"conan exited with code {completedProcess.returncode}\nconanfile: {conanfile}")


def GenerateProjectCMakeLists():
    for project in ALL_PROJECTS:
        CMakeListsPath = os.path.join(SCRIPT_DIR, "Source", project.directory, "CMakeLists.txt")
        CMakeListsFile = open(CMakeListsPath, "w")
        CMakeListsFile.write("# GENERATED FILE: See Setup.py\n")
        CMakeListsFile.write(f"project({project.name} CXX)\n")

        ##### declare_*_project
        if isinstance(project, GameProject):
            engineDeps = []
            if project.engine_dependencies:
                for dep in project.engine_dependencies:
                    engineDeps.append(dep.name)
            joinedDeps = ";".join(engineDeps)
            CMakeListsFile.write(f"declare_game_project(\"{joinedDeps}\")\n")
        elif isinstance(project, EngineProject):
            CMakeListsFile.write(f"declare_engine_project()\n")

        ##### find_package
        if project.conan_dependencies:
            for dep in project.conan_dependencies:
                CMakeListsFile.write(f"find_package({dep.cmake_find} REQUIRED)\n")  

        ##### collect deps
        public_deps = []
        if project.engine_dependencies:
            for dep in project.engine_dependencies:
                if dep.is_public:
                    public_deps.append(dep.name)
        if project.conan_dependencies:
            for dep in project.conan_dependencies:
                if dep.is_public:
                    public_deps.append(dep.cmake_name)

        private_deps = []
        if project.engine_dependencies:
            for dep in project.engine_dependencies:
                if not dep.is_public:
                    private_deps.append(dep.name)
        if project.conan_dependencies:
            for dep in project.conan_dependencies:
                if not dep.is_public:
                    private_deps.append(dep.cmake_name)

        ##### target_link_libraries
        if public_deps or private_deps:
            CMakeListsFile.write("target_link_libraries(${PROJECT_NAME}\n")
            if public_deps:
                CMakeListsFile.write("    PUBLIC\n")
                for dep in public_deps:
                    CMakeListsFile.write(f"        {dep}\n")
            if private_deps:
                CMakeListsFile.write("    PRIVATE\n")
                for dep in private_deps:
                    CMakeListsFile.write(f"        {dep}\n")
            CMakeListsFile.write(")\n")
        CMakeListsFile.close()

def SetupCMake(buildType : str):
    import subprocess
    cmakeSourceDir = os.path.join(SCRIPT_DIR, "Source")
    cmakeBuildDir = GetCMakeTempDir(buildType)
    packageInstallDir = os.path.join(cmakeBuildDir, "Package")
    
    PrintContext(f"packageInstallDir: {packageInstallDir}")

    completedProcess = subprocess.run(["cmake",
                    "-S", cmakeSourceDir,
                    "-B", cmakeBuildDir,
                    "--preset", f"conan-default-{buildType.lower()}",
                    f"-DCMAKE_BUILD_TYPE={buildType}", 
                    f"-DCMAKE_CONFIGURATION_TYPES={buildType}",
                    f"-DCMAKE_INSTALL_PREFIX={packageInstallDir}"])
    if completedProcess.returncode != 0:
        PrintError(f"cmake exited with code {completedProcess.returncode}")

def TryMakeFileLink(src : str, dst : str):
    if os.path.exists(src):
        if os.path.exists(dst):
            if os.path.islink(dst):
                os.remove(dst)
            else:
                raise Exception(f"Tried to create a link, but the destination exists and is not a link. path: {dst}")
        os.symlink(src, dst, target_is_directory=False)

def SetupLinks(buildType : str):
    # Todo: setup links for each game in ALL_PROJECTS
    afexSlnSource = os.path.join(GetCMakeTempDir(buildType), f"source.sln")
    destName = f"source_{buildType.lower()}.sln"
    afexSlnDest = os.path.join(SCRIPT_DIR, destName)
    PrintContext(f"re-creating shortcut to {destName}")
    TryMakeFileLink(afexSlnSource, afexSlnDest)
    if not os.path.exists(afexSlnDest) or not os.path.islink(afexSlnDest):
        PrintError(f"Failed to create shortcut to {destName}")

# At the time of writing conan 2 does not handle multi-configuration generators properly
# This means we need to split out debug and release configs for the time being.
# A consequence of this is the CMake preset 'conan-default' will be duplicated for each of our configs.
# We don't actually use the default target anyways but the conflict does cause issues with the specific
# preset (ie: conan-release) which would otherwise inherit 'conan-default' that has conflicting definitions.
# We correct this by appending the build type onto 'conan-default'. Example: 'conan-default-release'.
def FixupCMakeUserPresets(buildType : str):
    import json
    presetsPath = os.path.join(GetConanTempDir(buildType), "build", "generators", "CMakePresets.json")
    PrintContext(f"CMakePresets: {presetsPath}")
    with open(presetsPath, 'r') as presetsFile:
        jsonObject = json.load(presetsFile)
    name = jsonObject["configurePresets"][0]["name"]
    jsonObject["configurePresets"][0]["name"] = f"{name}-{buildType.lower()}"
    jsonObject["buildPresets"][0]["configurePreset"] = f"{name}-{buildType.lower()}"
    jsonObject["testPresets"][0]["configurePreset"] = f"{name}-{buildType.lower()}"
    with open(presetsPath, 'w') as presetsFile:
        json.dump(jsonObject, presetsFile, indent = 4)

BeginSection("setup.py")
PrintContext(f"SCRIPT_DIR: {SCRIPT_DIR}")

################################################################################
BeginSection("check for tools")
CheckForTools()
EndSection("check for tools")

################################################################################
BeginSection("directory setup")
SetupDirectories()
EndSection("directory setup")

################################################################################
BeginSection("Generate CMakeLists.txt")
GenerateProjectCMakeLists()
EndSection("Generate CMakeLists.txt")

################################################################################
BeginSection("conan")

for buildType in BUILD_CONFIGS:
    BeginSection(f"conan - {buildType.lower()}")
    SetupConan(buildType)
    EndSection(f"conan - {buildType.lower()}")

BeginSection("Fixup CMakePresets")
for buildType in BUILD_CONFIGS:
    FixupCMakeUserPresets(buildType)
EndSection("fixup CMakePresets")

EndSection("conan")

################################################################################
BeginSection("cmake")

for buildType in BUILD_CONFIGS:
    BeginSection(f"cmake - {buildType.lower()}")
    SetupCMake(buildType)
    EndSection(f"cmake - {buildType.lower()}")

EndSection("cmake")

################################################################################
BeginSection("link setup")
for buildType in BUILD_CONFIGS:
    SetupLinks(buildType)

EndSection("link setup")

EndSection("setup.py")