import argparse
import os
import shutil
import pefile
import fnmatch
import tomli
import tomli_w
from DevEnvironment.BuildFormatting import BeginSection, EndSection, PrintContext, PrintError
from Source.Projects import GetAllProjects, GameProject, ProjectBase

# Easily Configurable:
TEMP_DIR_NAME = "Generated"
POSSIBLE_CONFIGS = ["Debug", "Release"]
POSSIBLE_TARGETS = [project.name for project in GetAllProjects() if isinstance(project, GameProject)]

# Advanced:
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TEMP_DIR = os.path.join(SCRIPT_DIR, TEMP_DIR_NAME)
PACKAGE_DIR_ROOT = os.path.join(TEMP_DIR, "Packages")

# Globals:
gPackagesCreated=[]

parser = argparse.ArgumentParser(
    prog="Package.py",
    description="AFEX packaging script."
)
parser.add_argument("-c", "--configs", nargs='+', choices=POSSIBLE_CONFIGS, default=POSSIBLE_CONFIGS)
parser.add_argument("-t", "--targets", nargs='+', choices=POSSIBLE_TARGETS, default=POSSIBLE_TARGETS)
args = parser.parse_args()
gBuildConfigs = args.configs
gTargets = args.targets

def GetCMakeTempDir(buildType : str):
    return os.path.join(TEMP_DIR, f"CMake_{buildType}")

def remove_extension(path):
    root, ext = os.path.splitext(path)
    return root

def get_imported_dlls(executable_path, exclude_patterns=None):
    try:
        pe = pefile.PE(executable_path)

        # Check if the file has an import table
        if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
            dlls = []
            for entry in pe.DIRECTORY_ENTRY_IMPORT:
                dll_name=entry.dll.decode('utf-8')
                if exclude_patterns:
                    if any(fnmatch.fnmatch(dll_name, pattern) for pattern in exclude_patterns):
                        continue
                dlls.append(dll_name)
            return dlls
        else:
            return None

    except pefile.PEFormatError as e:
        print(f"Error parsing PE file: {e}")
        return None

def find_file_recursively(root_directory, target_filename):
    matches = []
    for dirpath, dirnames, filenames in os.walk(root_directory):
        if target_filename in filenames:
            matches.append(os.path.join(dirpath, target_filename))
    if len(matches) == 0:
        return None
    return matches

# Looks at the import table for the target peFile and searches for its dependencies.
# Those dependencies are copied to the package output directory and then we repeat that process for each of those dependencies
def CopyTargetDllsRecursively(
        target:str, config:str,
        peFile:str, destinationDir:str):
    global gPackagesCreated
    cmakeDir = GetCMakeTempDir(config)
    exclude_patterns = [
        'KERNEL32.dll',
        'VCRUNTIME*.dll',
        'ucrtbased.dll',
        'msvc*.dll',
        'USER32.dll',
        'GDI32.dll',
        'SHELL32.dll',
        'OPENGL32.dll',
        'api-ms-*.dll']
    dependentDlls = get_imported_dlls(peFile, exclude_patterns)
    if dependentDlls:
        print("Imported DLLs:")
        for dll in dependentDlls:
            print(f"looking for {dll}")
            foundFiles = find_file_recursively(os.path.join(cmakeDir, "Engine"), dll)
            if(foundFiles):
                if len(foundFiles) > 1:
                    PrintError(f"Multiple candidates for {dll} found in Engine.");
                else:
                    shutil.copy2(foundFiles[0], destinationDir)
                    CopyTargetDllsRecursively(target, config, foundFiles[0], destinationDir)
            else:
                foundFiles = find_file_recursively(os.path.join(cmakeDir, "full_deploy"), dll)
                if(foundFiles):
                    if len(foundFiles) > 1:
                        PrintError(f"Multiple candidates for {dll} found in full_deploy (staging directory for conan dlls).");
                    else:
                        shutil.copy2(foundFiles[0], destinationDir)
                        CopyTargetDllsRecursively(target, config, foundFiles[0], destinationDir)

def CopyAFEXConfigFiles(targetProject:ProjectBase, destinationPackage:str):
    afexConfigsRoot = os.path.join(SCRIPT_DIR, "Source", targetProject.directory, "Configs")
    if os.path.exists(afexConfigsRoot) and os.path.isdir(afexConfigsRoot):
        shutil.copytree(afexConfigsRoot, destinationPackage, dirs_exist_ok=True)

# This function will perform afex path string replacements similar to how they're done in engine
# a few differences, though:
#   1) far less error checking, it's assumed the dev isn't doing anything crazy
#   2) Only {{app}} and {{cwd}} are respected. They are assumed to be the executable path
def ResolveAndCopyAssetDirectory(destinationAssetsDir:str, configuredAssetsDir:str, target:str, config:str):
    cmakeDir = GetCMakeTempDir(config)
    targetDir = os.path.join(cmakeDir, target)
    appPath = os.path.join(targetDir, config)

    # todo: handle app/cwd replacements case insensitively
    if configuredAssetsDir.find("./") == 0 or configuredAssetsDir.find(".\\") == 0:
        configuredAssetsDir = appPath + configuredAssetsDir[1:]
    elif configuredAssetsDir.find("{{app}}") != -1:
        configuredAssetsDir = configuredAssetsDir.replace("{{app}}", appPath)
    elif configuredAssetsDir.find("{{cwd}}") != -1:
        configuredAssetsDir = configuredAssetsDir.replace("{{cwd}}", appPath)

    if os.path.exists(configuredAssetsDir) and os.path.isdir(configuredAssetsDir):
        shutil.copytree(configuredAssetsDir, destinationAssetsDir, dirs_exist_ok=True)

def CopyAssets(destinationPackage:str, target:str, config:str):
    # this assumes we successfully copied the engine config from a Configs directory already
    engineConfigPath = os.path.join(destinationPackage, "afex.toml")

    if not os.path.exists(engineConfigPath):
        PrintError(f"Couldn't find afex.toml in the destination package folder for {target} {config}. "
            "that probably indicates there wasn't an afex.toml file in a directory named Configs "
            "in the source directory of that project. This won't break the package but might indicate a mistake.")
        return
    
    afexConfig:dict[str,any]=None
    with open(engineConfigPath, 'rb') as file:
        try:
            afexConfig = tomli.load(file)
        except tomli.TOMLDecodeError:
            PrintError("Failed to decode the engine config at {engineConfigPath}.\n"
                       "This is probably an unrecoverable error but we will try to continue.")
            return
        
        if ('filesystem' not in afexConfig) or (not isinstance(afexConfig['filesystem'], dict)):
            PrintError(f"The engine config for {target} {config} did not contain a dictionary named filesystem")
            return
        
        if ('asset_directories' not in afexConfig['filesystem']) or (not isinstance(afexConfig['filesystem']['asset_directories'], list)):
            PrintError(f"The engine config for {target} {config} did not contain a list of asset_directories under filesystem")
            return
        
        assetDirectories = afexConfig['filesystem']['asset_directories']
        for assetDir in assetDirectories:
            ResolveAndCopyAssetDirectory(os.path.join(destinationPackage, "Assets"), assetDir, target, config)
        
    # Update the engine config pointing assets only locally to the package
    afexConfig['filesystem']['asset_directories'] = ["{{app}}/Assets"]
    with open(engineConfigPath, 'wb') as file:
        tomli_w.dump(afexConfig, file)


def PackageTargetAndConfig(target:str, config:str):
    global gPackagesCreated

    targetProject:ProjectBase=None
    for project in GetAllProjects():
        if project.name == target:
            targetProject = project
            break

    cmakeDir = GetCMakeTempDir(config)
    targetDir = os.path.join(cmakeDir, target)
    # The following root contains all packaged output including debug and log information.
    # The root that is shippable to consumers should be under this path in a target-named directory
    destinationRoot = os.path.join(PACKAGE_DIR_ROOT, target, config)
    # this is the path that should be shippable to customers
    destinationPackage = os.path.join(destinationRoot, target)
    if not os.path.exists(targetDir):
        PrintContext(f"Target ({target}) Config({config}) not built")
        return
    if not os.path.isdir(targetDir):
        PrintError(f"Target path is not a directory: {targetDir}")
        return
    targetExe = os.path.join(targetDir, config, f"{target}.exe")
    targetPdb = os.path.join(targetDir, config, f"{target}.pdb")
    PrintContext(f"Target exe: {targetExe}")
    PrintContext(f"Target pdb: {targetPdb}")
    if not os.path.exists(targetExe):
        PrintError("Target executable not found")
        return
    if not os.path.exists(targetPdb):
        PrintContext("Target pdb not found")
        # don't exit here. We can continue without a pdb
    os.makedirs(destinationPackage, exist_ok=True)
    shutil.copy2(targetExe, destinationPackage)
    if os.path.exists(targetPdb):
        # The PDB doesn't ship to customers so place it in a destination different (and not under) the path
        # where the executable is being placed.
        shutil.copy2(targetPdb, destinationRoot)


    CopyAFEXConfigFiles(targetProject, destinationPackage)

    CopyAssets(destinationPackage, target, config)

    CopyTargetDllsRecursively(target, config, peFile=targetExe, destinationDir=destinationPackage)

    gPackagesCreated.append({'config':config,'target':target})

def FindPackageResultFromConfigAndTarget(config, target):
    global gBuildConfigs
    for createdPackage in gPackagesCreated:
        if createdPackage['config']==config and createdPackage['target'] == target:
            return createdPackage
    return None

BeginSection("Package")
for target in gTargets:
    for config in gBuildConfigs:
        BeginSection(f"Package - {config}")
        PackageTargetAndConfig(target, config)
        EndSection(f"Package - {config}")
BeginSection("summary")
for possibleTarget in POSSIBLE_TARGETS:
    for possibleConfig in POSSIBLE_CONFIGS:
        targetIsSelected = possibleTarget in gTargets
        configIsSelected = possibleConfig in gBuildConfigs
        if not targetIsSelected or not configIsSelected:
            PrintContext(f"{possibleTarget}-{possibleConfig} not selected for packaging")
        else:
            result = FindPackageResultFromConfigAndTarget(possibleConfig, possibleTarget)
            if result:
                PrintContext(f"{possibleTarget}-{possibleConfig} packaged!")
            else:
                PrintError(f">>  {possibleTarget}-{possibleConfig} not packaged")
EndSection("summary")
EndSection("Package")