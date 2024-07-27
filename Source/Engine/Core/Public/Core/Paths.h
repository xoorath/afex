#pragma once

// Engine
#include <Core/Core.export.h>

// System
#include <filesystem>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    //////////////////////////////////////////////////////////////////////////
    // The paths singleton identifies useful files and directories for the
    // application.
    class Paths
    {
    public:
        // Construct once in the entry point.
        CORE_EXPORT Paths(int argc, char* argv[]);
        CORE_EXPORT ~Paths(); /*=default*/

        CORE_EXPORT static std::filesystem::path ExecutableDirectory();
        CORE_EXPORT static std::filesystem::path ExecutablePath();

    private:
        Paths()                                     = delete;
        Paths(const Paths&)                         = delete;
        Paths(Paths&&) noexcept                     = delete;
        Paths& operator=(const Paths&)              = delete;
        Paths& operator=(Paths&&) noexcept          = delete;
    
        static std::filesystem::path s_ExecutableDirectory;
        static std::filesystem::path s_ExecutablePath;
    };
}