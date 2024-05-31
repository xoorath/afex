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

def GenerateProjectCMakeLists(project: ProjectBase):
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

def GenerateRootCMakeLists(intermediateDirectories : list[str]):
    CMakeListsPath = os.path.join(SCRIPT_DIR, "Source", "CMakeLists.txt")
    CMakeListsFile = open(CMakeListsPath, "w")
    CMakeListsFile.write("# GENERATED FILE: See Setup.py\n")
    CMakeListsFile.write("cmake_minimum_required(VERSION 3.6)\n")
    CMakeListsFile.write("project(Source CXX)\n")
    CMakeListsFile.write("include(../DevEnvironment/CMake/common_project.cmake)\n")
    CMakeListsFile.write("include(../DevEnvironment/CMake/game_project.cmake)\n")
    CMakeListsFile.write("include(../DevEnvironment/CMake/engine_project.cmake)\n")
    CMakeListsFile.write("include(../DevEnvironment/CMake/third_party.cmake)\n")
    CMakeListsFile.write("include(GenerateExportHeader)\n")
    for dir in intermediateDirectories:
        dirSplit = str.split(os.sep)
        if len(dirSplit) == 1:
            CMakeListsFile.write(f"add_subdirectory({dir})\n")

    CMakeListsFile.write("if(MSVC)\n")
    CMakeListsFile.write("    set_property(GLOBAL PROPERTY USE_FOLDERS ON)\n")
    startupProj = None
    for project in ALL_PROJECTS:
        if isinstance(project, GameProject) and project.startup:
            startupProj = project.name
            break
    if startupProj:
        CMakeListsFile.write(f"    set_property(DIRECTORY ${{CMAKE_CURRENT_SOURCE_DIR}} PROPERTY VS_STARTUP_PROJECT {startupProj})\n")
    CMakeListsFile.write("    set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER \"2 CMake\")\n")
    CMakeListsFile.write("endif()\n")
    CMakeListsFile.close()

def GenerateCMakeLists():
    # this set of all project directories will help us find which intermediate directories need CMakeLists
    # For example: Engine requires a CMakeLists.txt even though it itself is not a project dir.
    projectDirectories=[]
    absProjectDirectories=[] # same as above, but the absolute path
    # This set of directories within Source contain a project
    # These will be included directly by the CMakeLists.txt in the Source directory
    topLevelProjectDirectories=[]
    for project in ALL_PROJECTS:
        GenerateProjectCMakeLists(project)
        dirParts = project.directory.split('/')
        dir = '/'.join(dirParts)
        if not dirParts[0] in topLevelProjectDirectories:
            topLevelProjectDirectories.append(dirParts[0])
        if not dir in projectDirectories:
            projectDirectories.append(dir)
            absProjectDirectories.append(os.path.abspath(os.path.join(SCRIPT_DIR, "Source", dir)))
    GenerateRootCMakeLists(topLevelProjectDirectories)
    
    intermediateDirectories=[]
    absIntermediateDirectories = []
    for dir in projectDirectories:
        partialDir = "";
        for dirPart in dir.split('/'):
            partialDir += dirPart
            if partialDir not in intermediateDirectories and partialDir not in projectDirectories:
                intermediateDirectories.append(partialDir)
                absIntermediateDirectories.append(os.path.abspath(os.path.join(SCRIPT_DIR, "Source", partialDir)))
            partialDir += '/'

    for intDir in intermediateDirectories:
        CMakeListsPathRoot = os.path.join(SCRIPT_DIR, "Source", intDir)
        CMakeListsPath = os.path.join(CMakeListsPathRoot, "CMakeLists.txt")
        CMakeListsFile = open(CMakeListsPath, "w")
        CMakeListsFile.write("# GENERATED FILE: See Setup.py\n")
        CMakeListsFile.write(f"project({intDir})\n")
        for child in os.listdir(CMakeListsPathRoot):
            childPath = os.path.join(CMakeListsPathRoot, child)
            if os.path.isdir(childPath):
                isProjectDir = False
                for i in range(len(absProjectDirectories)):
                    if childPath == absProjectDirectories[i]:
                        isProjectDir = True
                        splitProjDir = projectDirectories[i].split('/')
                        CMakeListsFile.write(f"add_subdirectory({splitProjDir[len(splitProjDir)-1]})\n")
                        break
                if not isProjectDir:
                    for i in range(len(absIntermediateDirectories)):
                        if childPath == absIntermediateDirectories[i]:
                            splitIntDir = intermediateDirectories[i].split('/')
                            CMakeListsFile.write(f"add_subdirectory({splitIntDir[len(splitIntDir)-1]})\n")
                            break
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
GenerateCMakeLists()
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