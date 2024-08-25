// Core
#include <Core/Assert.h>
#include <Core/Paths.h>
#include <Core/Text.h>

// System
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <format>
#include <future>
#include <sstream>
#include <system_error>
#include <random>
// System - Windows
#ifdef _WIN32
#include <ShlObj.h> // requires shell32.lib
#pragma comment(lib, "shell32.lib")
#endif

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////
namespace
{

}

//////////////////////////////////////////////////////////////////////////
namespace Core::Paths
{
    ////////////////////////////////////////////////////////////////////////// Internal
    namespace
    {
        static std::atomic_bool s_InitializedOnce = false;
        static fs::path s_ApplicationPath;
        static fs::path s_ApplicationDirectory;
        static fs::path s_CurrentWorkingDirectory;
        static fs::path s_LocalDirectory;
        static fs::path s_RoamingDirectory;
        static fs::path s_TempDirectory;

        void CheckImportantAppDirectory(const fs::path& path, std::string_view variableName, std::string_view humanName)
        {
            std::error_code ec;
            if (!fs::exists(path, ec))
            {
                // Try to create that directory.
                AFEX_FATAL_CHECK_MSG(fs::create_directories(path, ec),
                    "Could not create {}.\n{}: {}\nerror: {}", humanName, variableName, path.string(), ec.message());
            }
            else
            {
                AFEX_FATAL_CHECK_MSG(fs::is_directory(path, ec),
                    "{} is not a directory.\n{}: {}\nerror: {}", humanName, variableName, path.string(), ec.message());
            }
            const fs::perms p = fs::status(path, ec).permissions();

            AFEX_FATAL_CHECK_MSG(ec.value() == 0,
                "The application failed to get the permissions of {}.\n{}: {}\nerror: {}",
                humanName, variableName, path.string(), ec.message());

            AFEX_FATAL_CHECK_MSG((p & fs::perms::owner_read) != fs::perms::none,
                "The application does not have permissions to read {}.\n{}: {}\nerror: {}",
                humanName, variableName, path.string(), ec.message());

            AFEX_FATAL_CHECK_MSG((p & fs::perms::owner_write) != fs::perms::none,
                "The application does not have permissions to write {}.\n{}: {}\nerror: {}",
                humanName, variableName, path.string(), ec.message());
        }

        std::string GenerateUUID() {
            std::random_device rd; // Obtain a random number from hardware
            std::mt19937 gen(rd()); // Seed the generator

            std::uniform_int_distribution<> dis(0, 15); // Define the range

            std::ostringstream oss;
            oss << std::hex;
            for (int32_t i = 0; i < 8; ++i)
            {
                oss << dis(gen);
            }
            oss << "-";
            for (int32_t i = 0; i < 4; ++i)
            {
                oss << dis(gen);
            }
            oss << "-";
            for (int32_t i = 0; i < 4; ++i)
            {
                oss << dis(gen);
            }
            oss << "-";
            for (int32_t i = 0; i < 4; ++i)
            {
                oss << dis(gen);
            }
            oss << "-";
            for (int32_t i = 0; i < 12; ++i)
            {
                oss << dis(gen);
            }

            return oss.str();
        }
    }

    ////////////////////////////////////////////////////////////////////////// Public
    /*CORE_EXPORT*/ bool GlobalInit(int argc, const char* argv[])
    {
        using namespace std::string_view_literals;

        bool initExpected = false;
        if(!s_InitializedOnce.compare_exchange_strong(initExpected, true))
        {
            AFEX_ASSERT_FAIL("Core::Paths::EarlyInit already called.");
            return false;
        }

        AFEX_ASSERT(argc > 0);
        s_ApplicationPath = fs::path(argv[0]);
        AFEX_LOG_TRACE("Core::Paths - s_ApplicationPath = {}", s_ApplicationPath.string());

        AFEX_ASSERT(s_ApplicationPath.has_parent_path());
        s_ApplicationDirectory = s_ApplicationPath.parent_path();
        AFEX_LOG_TRACE("Core::Paths - s_ApplicationDirectory = {}", s_ApplicationDirectory.string());

        s_CurrentWorkingDirectory = std::filesystem::current_path();

        // The application name without extension will be used for local and roaming app locations
        std::string applicationName = s_ApplicationPath.filename().replace_extension(fs::path()).string();
        AFEX_LOG_TRACE("Core::Paths - applicationName = {}", applicationName);

#if defined(_WIN32) || defined(_WIN64)
        PWSTR path = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path)))
        {
            AFEX_LOG_TRACE("Core::Paths - FOLDERID_LocalAppData = {}", Core::Text::string_cast(path).c_str());
            s_LocalDirectory = fs::path(path) / applicationName;
            AFEX_LOG_TRACE("Core::Paths - s_LocalDirectory = {}", s_LocalDirectory.string());
            CoTaskMemFree(path);
        }
        else
        {
            AFEX_ASSERT_FAIL("Failed to get the user's app local directory.");
        }
#else
        const char* homeDir = std::getenv("HOME");
        AFEX_LOG_TRACE("Core::Paths - HOME = {}", homeDir);
        AFEX_FATAL_CHECK_MSG(homeDir != nullptr, "The home directory could not be found with an environment variable named HOME.");
        s_LocalDirectory = std::filesystem::path(homeDir) / ".local" / "share" / applicationName;
        AFEX_LOG_TRACE("Core::Paths - s_LocalDirectory = {}", s_LocalDirectory.string());
#endif

        CheckImportantAppDirectory(s_LocalDirectory, "s_LocalDirectory"sv, "the user's app local directory"sv);

#if defined(_WIN32) || defined(_WIN64)
        path = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path)))
        {
            AFEX_LOG_TRACE("Core::Paths - FOLDERID_RoamingAppData = {}", Core::Text::string_cast(path).c_str());
            s_RoamingDirectory = fs::path(path) / applicationName;
            AFEX_LOG_TRACE("Core::Paths - s_RoamingDirectory = {}", s_RoamingDirectory.string());
            CoTaskMemFree(path);
        }
        else
        {
            AFEX_ASSERT_FAIL("Failed to get the user's app roaming directory.");
        }
#else
        s_RoamingDirectory = std::filesystem::path(homeDir) / ".config" / applicationName;
        AFEX_LOG_TRACE("Core::Paths - s_RoamingDirectory = {}", s_RoamingDirectory.string());
#endif

        CheckImportantAppDirectory(s_RoamingDirectory, "s_RoamingDirectory"sv, "the user's app roaming directory"sv);

        s_TempDirectory = fs::temp_directory_path() / std::format("{}_{}", applicationName, GenerateUUID());
        AFEX_LOG_TRACE("Core::Paths - s_TempDirectory = {}", s_TempDirectory.string());

        CheckImportantAppDirectory(s_TempDirectory, "s_TempDirectory"sv, "a temporary app directory"sv);

        return true;
    }

    /*CORE_EXPORT*/ bool GlobalShutdown(std::chrono::seconds timeout)
    {
        s_ApplicationPath.clear();
        s_ApplicationDirectory.clear();
        s_CurrentWorkingDirectory.clear();
        s_LocalDirectory.clear();
        s_RoamingDirectory.clear();

        if (std::filesystem::exists(s_TempDirectory))
        {
            const auto deleteDirectoryTask = [](const std::filesystem::path& directory, std::promise<std::error_code>&& promise)
                {
                    std::error_code ec;
                    std::filesystem::remove_all(directory, ec);
                    promise.set_value(ec);
                };

            std::promise<std::error_code> promise;
            std::future<std::error_code> future = promise.get_future();

            // Start the delete task asynchronously
            std::thread deleteDirectoryThread = std::thread(deleteDirectoryTask, s_TempDirectory, std::move(promise));
            deleteDirectoryThread.detach();

            // Wait for the result with a timeout
            if (future.wait_for(timeout) == std::future_status::ready)
            {
                std::error_code ec = future.get();
                if (ec.value() == 0)
                {
                    AFEX_LOG_TRACE("Temporary directory deleted.\ns_TempDirectory: {}", s_TempDirectory.string());
                }
                else
                {
                    AFEX_LOG_ERROR("Failed to delete temporary directory.\ns_TempDirectory: {}\nError: {}", s_TempDirectory.string(), ec.message());
                    s_TempDirectory.clear();
                    s_InitializedOnce = false;
                    return false;
                }
            }
            else
            {
                AFEX_LOG_WARNING("Delete temp directory operation timed out. Moving on without deleting.");
                s_TempDirectory.clear();
                s_InitializedOnce = false;
                return false;
            }
        }
        else
        {
            AFEX_LOG_TRACE("Temporary directory doesn't exist. Nothing to delete.\ns_TempDirectory: {}", s_TempDirectory.string());
        }
        s_TempDirectory.clear();
        s_InitializedOnce = false;
        return true;
    }

    /*CORE_EXPORT*/ const fs::path& ApplicationPath()
    {
        return s_ApplicationPath;
    }

    /*CORE_EXPORT*/ const fs::path& ApplicationDirectory()
    {
        return s_ApplicationDirectory;
    }

    /*CORE_EXPORT*/ const fs::path& CurrentWorkingDirectory()
    {
        return s_CurrentWorkingDirectory;
    }

    /*CORE_EXPORT*/ const fs::path& LocalDirectory()
    {
        return s_LocalDirectory;
    }

    /*CORE_EXPORT*/ const fs::path& RoamingDirectory()
    {
        return s_RoamingDirectory;
    }

    /*CORE_EXPORT*/ const fs::path& TempDirectory()
    {
        return s_TempDirectory;
    }
}