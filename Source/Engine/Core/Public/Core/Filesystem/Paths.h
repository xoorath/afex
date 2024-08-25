#pragma once

// Engine
#include <Core/Core.export.h>

// System
#include <filesystem>
#include <string_view>

//////////////////////////////////////////////////////////////////////////
namespace Core::Paths
{
    // Early init should be setup once before the engine starts but
    // after the global logger is configured.
    //
    // Returns true if paths were setup correctly. Expect this to typically
    // or always return true as a failure to setup paths is considered fatal.
    CORE_EXPORT bool GlobalInit(int argc, const char* argv[]);

    // Shutdown should be called once to terminate pathing functionality.
    // This will attempt to destroy the temporary directory with a timeout.
    // After the timeout has expired if we could not destroy the temporary directory
    // execution will continue and the thread attempting deletion will be detached. 
    //
    // Returns true if the temporary directory did not exist or was deleted successfully.
    CORE_EXPORT bool GlobalShutdown(std::chrono::seconds timeout = std::chrono::seconds(10));

    // Gets the path to the application executable
    CORE_EXPORT const std::filesystem::path& ApplicationPath();

    // The directory where the application executable is found
    CORE_EXPORT const std::filesystem::path& ApplicationDirectory();

    // The location the application was launched from.
    // In AFEX the current working directory is set at startup and not queried
    // at runtime. To get the actual changing current working directory use 
    // the standard library's std::filesystem::current_path.
    //
    // When using relative directories with no specified root in AFEX it's
    // assumed that the path is relative to this directory even if the actual
    // current working directory is changed by the application at runtime.
    CORE_EXPORT const std::filesystem::path& CurrentWorkingDirectory();

    // The users local directory specific to this application 
    // Example: "C:\Users\username\AppData\Local\appname\"
    // Note: appname will use the executable name without file extension
    // 
    // The local directory is appropriate for saving files specific to this 
    // user and machine It can also be used for larger files that are
    // inappropriate for syncing across devices
    CORE_EXPORT const std::filesystem::path& LocalDirectory();

    // The users roaming directory specific to this application 
    // Example: "C:\Users\username\AppData\Roaming\appname\"
    // Note: appname will use the executable name without file extension
    // 
    // The roaming directory is appropriate for saving files specific to this
    // user that they may want synced across devices such as bookmarks, game
    // progress and preferences.
    CORE_EXPORT const std::filesystem::path& RoamingDirectory();

    // A temporary directory that can be used during this session.
    // Example: "C:\Users\username\AppData\Local\Temp\unique_name\"
    //
    // This Paths class will attempt to destroy this directory when it's
    // destructed.
    // It's still a good practice to delete each temporary file when it is no
    // longer needed so this process can complete quickly.
    CORE_EXPORT const std::filesystem::path& TempDirectory();
}