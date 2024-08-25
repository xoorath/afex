#pragma once

// Engine
#include <Core/Text.h>

// System
#include <cstdint>
#include <filesystem>
#include <map>
#include <string_view>
#include <vector>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    class Config;
    enum class ResolvePathResult : uint8_t;

    //////////////////////////////////////////////////////////////////////////
    class FilesystemImpl
    {
    public:
        explicit FilesystemImpl(Core::Config& filesystemConfig);

        FilesystemImpl() = delete;
        FilesystemImpl(const FilesystemImpl&);                  /*= default*/
        FilesystemImpl(FilesystemImpl&&) noexcept;              /*= default*/
        FilesystemImpl& operator=(const FilesystemImpl&);       /*= default*/
        FilesystemImpl& operator=(FilesystemImpl&&) noexcept;   /*= default*/
        ~FilesystemImpl();                                      /*= default*/

        // Creates an absolute path from a path that may be relative and/or contain variables.
        ResolvePathResult ResolvePath(const std::filesystem::path& pathIn, std::filesystem::path& outPath) const;

        // Returns true if the result arg is any error type
        static bool ResolvePathResultIsError(ResolvePathResult result);

        // Get a human readable string for the path result
        static const std::string_view ResolvePathResultToString(ResolvePathResult result);


        // Special named variables that can be substituted when resolving a path.
        // Only one of these variables may be used and must always be the first part of a path.
        static const std::string_view s_App;
        static const std::string_view s_Assets;
        static const std::string_view s_Cwd;
        static const std::string_view s_Local;
        static const std::string_view s_Roaming;
        static const std::string_view s_Temp;

        using VarMap = std::map<std::string_view, std::filesystem::path,
            Core::Text::Comparitor<Core::Text::OrderingRules::CaseInsensitive>>;
    private:

        // An ordered collection of asset directories.
        // The first asset directory will be used first when resolving asset paths.
        std::vector<std::filesystem::path> m_AssetDirectories;

        // Maps variable names (s_App, ...etc) to paths at startup
        // these values are assumed to not change after the filesystem is created.
        // The assets path is not mapped as it's not a constant 1:1 relationship
        VarMap m_VariableMap;
    };
}