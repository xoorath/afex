import argparse
import os
import shutil
import pefile
import fnmatch
from DevEnvironment.BuildFormatting import BeginSection, EndSection, PrintContext, PrintError

# Easily Configurable:
TEMP_DIR_NAME = "Generated"
PROJECT_ROOTS = [ "Source" ]
POSSIBLE_CONFIGS = ["Debug", "Release"]
POSSIBLE_TARGETS = ["HelloWorld"]

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


def PackageTargetAndConfig(target:str, config:str):
    global gPackagesCreated
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