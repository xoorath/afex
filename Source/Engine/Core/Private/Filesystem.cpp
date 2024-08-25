#include <Core/Filesystem.h>

// Engine
#include <Core/Assert.h>
#include <Core/Config/Config.h>
#include <Core/Logging.h>
#include <Core/Text.h>
#include <Core/Paths.h>

// System
#include <regex>
#include <string_view>
#include <map>

using namespace std::string_view_literals;
using namespace Core::Text;
namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    ////////////////////////////////////////////////////////////////////////// Internal
    namespace
    {
        enum class ResolvePathSegmentResult
        {
            Success,
            ErrorWhitespacePlacement,
            ErrorVariableConcatenated,
            ErrorVariablePlacement
        };

        std::string replaceEnvVariables(const std::string& str)
        {
            std::string result;
            std::regex varRegex("\\$\\{(\\w+)\\}|\\$(\\w+)");
            std::smatch match;
            auto start = str.cbegin();
            auto end = str.cend();

            while (std::regex_search(start, end, match, varRegex))
            {
                // Append the part of the string before the match
                result.append(start, match[0].first);

                // Extract the environment variable name
                std::string varName = match[1].matched ? match[1].str() : match[2].str();
                const char* varValue = std::getenv(varName.c_str());

                // Append the value of the environment variable or an empty string if not found
                if (varValue)
                {
                    result.append(varValue);
                }

                // Move past the current match
                start = match[0].second;
            }

            // Append the remaining part of the string
            result.append(start, end);

            return result;
        }

        ResolvePathSegmentResult TryReplacePathSegmentWithVariables(
            const Filesystem::VarMap& varMap,
            std::string_view fullPath,
            std::string_view strIn,
            fs::path& pathOut,
            bool isFirstSegment)
        {
            ////////////////////////////////////////////////////////////////////////// Check for whitespace issues
            if(strIn != TrimRight(strIn))
            {
                AFEX_LOG_ERROR("A path segment was found with whitespace at the end of the segment.\n"
                    "Leading whitespace is legal but not very well supported in applications. It is probably a mistake.\n"
                    "Trailing whitespace is not supported on all supported platforms and is therefore not supported in AFEX.\n"
                    "Path segment = \"{}\"\n"
                    "Path = \"{}\"", strIn, fullPath);
                return ResolvePathSegmentResult::ErrorWhitespacePlacement;
            }
#if AFEX_LOG_WARNING_ENABLED
            else if(strIn != TrimLeft(strIn))
            {
                AFEX_LOG_WARNING("A path segment was found with whitespace at the beginning of the segment.\n"
                    "Leading whitespace is legal but not very well supported in applications. It is probably a mistake.\n"
                    "Trailing whitespace is not supported on all supported platforms and is therefore not supported in AFEX.\n"
                    "Path segment = \"{}\"\n"
                    "Path = \"{}\"", strIn, fullPath);
            }
#endif

            ////////////////////////////////////////////////////////////////////////// Check for AFEX variables (except s_Assets)
            Filesystem::VarMap::const_iterator foundVar = varMap.find(strIn);
            if(foundVar != varMap.end())
            {
                if(isFirstSegment)
                {
                    pathOut = foundVar->second;
                    return ResolvePathSegmentResult::Success;
                }
                else
                {
                    AFEX_LOG_ERROR("Invalid path segment. AFEX filesystem variables can only be used at the beginning of a path.\n"
                        "Path segment = \"{}\"\n"
                        "Path = \"{}\"", strIn, fullPath);
                    return ResolvePathSegmentResult::ErrorVariablePlacement;
                }
            }

            ////////////////////////////////////////////////////////////////////////// Check for AFEX variables used improperly
            // Check for known variable names concatenated to string parts. 
            // That is not allowed.
            for(auto const& kvp : varMap)
            {
                if(ContainsCaseInsensitive(strIn, kvp.first))
                {
                    AFEX_LOG_ERROR("Invalid path segment. AFEX filesystem variables can only be used at the beginning of a path "
                        "and are only valid when not concatenated with other text.\n"
                        "For example: {}_foo/bar is invalid."
                        "Path segment = \"{}\"\n"
                        "Path = \"{}\"", Filesystem::s_App, strIn, fullPath);
                    return ResolvePathSegmentResult::ErrorVariableConcatenated;
                }
            }

            ////////////////////////////////////////////////////////////////////////// Check for environment variables with unix syntax
            // Note: we even use unix syntax on windows for resolving paths in AFEX

            pathOut = replaceEnvVariables(std::string(strIn));

            return ResolvePathSegmentResult::Success;
        }
    }

    ////////////////////////////////////////////////////////////////////////// Public
    /*static*/ const std::string_view Filesystem::s_App = "{{app}}"sv;
    /*static*/ const std::string_view Filesystem::s_Assets = "{{assets}}"sv;
    /*static*/ const std::string_view Filesystem::s_Cwd = "{{cwd}}"sv;
    /*static*/ const std::string_view Filesystem::s_Local = "{{local}}"sv;
    /*static*/ const std::string_view Filesystem::s_Roaming = "{{roaming}}"sv;
    /*static*/ const std::string_view Filesystem::s_Temp = "{{temp}}"sv;

    /*explicit CORE_EXPORT*/ Filesystem::Filesystem(Core::Config& filesystemConfig)
    {
        using namespace std::string_view_literals;

        m_VariableMap =
        {
            { Filesystem::s_App,        Core::Paths::ApplicationDirectory() },
            { Filesystem::s_Cwd,        Core::Paths::CurrentWorkingDirectory() },
            { "."sv,                    Core::Paths::CurrentWorkingDirectory() },
            { Filesystem::s_Local,      Core::Paths::LocalDirectory() },
            { Filesystem::s_Roaming,    Core::Paths::RoamingDirectory() },
            { Filesystem::s_Temp,       Core::Paths::TempDirectory() }
        };

        const std::vector<std::string> defaultAssetDirectories = 
            {
                std::string("{{app}}/Assets")
            };

        std::vector<std::string> assetDirectories = 
            filesystemConfig.GetSetting<std::vector<std::string>>("filesystem.asset_directories"sv, defaultAssetDirectories);

        AFEX_FATAL_CHECK_MSG(!assetDirectories.empty(), "AFEX was configured with 0 asset directories. This is probably a mistake"
            " and will cause the application to fail to load any files under {}", s_Assets);

        m_AssetDirectories.reserve(assetDirectories.size());

        for(std::string& str : assetDirectories)
        {
            if(str.empty())
            {
                AFEX_LOG_WARNING("An empty asset path was provided during filesystem startup.");
                continue;
            }

            if(ContainsCaseInsensitive(str, s_Assets))
            {
                AFEX_LOG_ERROR("An asset path can't contain {}.\nPath = \"{}\"", s_Assets, str);
                continue;
            }

            fs::path assetDirPath(str); // the path as it was provided
            fs::path assetDirPathTransformed; // the path after substitutions

            bool isFirstPathPart = true;
            for(const fs::path pathPart : assetDirPath)
            {
                fs::path transformedPathPart;
                auto result = TryReplacePathSegmentWithVariables(
                    /*varMap=*/         m_VariableMap,
                    /*fullpath=*/       str,
                    /*strIn=*/          pathPart.string(),
                    /*pathOut=*/        transformedPathPart,
                    /*isFirstSegment=*/ isFirstPathPart
                );
                isFirstPathPart = false;
                if(result == ResolvePathSegmentResult::Success)
                {
                    assetDirPathTransformed = assetDirPathTransformed / transformedPathPart;
                }
                else
                {
                    AFEX_LOG_ERROR("Invalid asset path provided during filesystem startup. This path will be ignored.\n"
                        "Path segment = \"{}\"\n"
                        "Path = \"{}\"", pathPart.string(), str);
                    continue;
                }
            }

            // Once the path has been built out with variables and environment variables
            // we then want to get the absolute path which will finish resolving any relative paths.
            assetDirPathTransformed = fs::absolute(assetDirPathTransformed);

#if AFEX_LOG_WARNING_ENABLED
            if(!fs::exists(assetDirPathTransformed))
            {
                AFEX_LOG_WARNING("An asset path that was provided during filesystem startup could not be found.\n"
                    "The path will be used regardless in case it is created later, but it may indicate an error.\n"
                    "Path = \"{}\"", assetDirPathTransformed.string());
            }
#endif
            m_AssetDirectories.push_back(assetDirPathTransformed);
        }

        AFEX_FATAL_CHECK_MSG(m_AssetDirectories.size(), "AFEX was configured with {} asset directories but none were valid.",
            assetDirectories.size());

#if AFEX_LOG_WARNING_ENABLED
        if(m_AssetDirectories.size() != assetDirectories.size())
        {
            AFEX_LOG_WARNING("AFEX was configured with {} asset directories but only {} will be used.\n"
            "See earlier log messages to understand why.", assetDirectories.size(), m_AssetDirectories.size());
        }
#endif

        bool anyExist = false;
        for(auto const& assetPath : m_AssetDirectories)
        {
            if(fs::exists(assetPath))
            {
                AFEX_LOG_TRACE("AFEX Filesystem asset directory (found): {}", assetPath.string());
                anyExist = true;
            }
            else
            {
                AFEX_LOG_TRACE("AFEX Filesystem asset directory (not found): {}", assetPath.string());
            }
        }

        AFEX_FATAL_CHECK_MSG(anyExist, "AFEX was configured with {} asset directories. None of these directories were found.",
            defaultAssetDirectories.size());
    }
    /*CORE_EXPORT*/ Filesystem::Filesystem(const Filesystem&) = default;
    /*CORE_EXPORT*/ Filesystem::Filesystem(Filesystem&&) noexcept = default;
    /*CORE_EXPORT*/ Filesystem& Filesystem::operator=(const Filesystem&) = default;
    /*CORE_EXPORT*/ Filesystem& Filesystem::operator=(Filesystem&&) noexcept = default;
    /*CORE_EXPORT*/ Filesystem::~Filesystem() = default;

    /*CORE_EXPORT static*/ bool Filesystem::ResolvePathResultIsError(Filesystem::ResolvePathResult result)
    {
        switch(result)
        {
        default:
            AFEX_ASSERT_FAIL("Unhandled resolve path result");
        case ResolvePathResult::ErrorEmpty:
        case ResolvePathResult::ErrorInvalid:
        case ResolvePathResult::ErrorMupltipleVariables:
        case ResolvePathResult::ErrorVariablePlacement:
        case ResolvePathResult::ErrorVariableConcatenated:
            return true;
        case ResolvePathResult::Resolved:
        case ResolvePathResult::ResolvedFound:
        case ResolvePathResult::ResolvedNotFound:
            return false;
        }
    }

    /*CORE_EXPORT static*/ const std::string_view Filesystem::ResolvePathResultToString(Filesystem::ResolvePathResult result)
    {
        using namespace std::string_view_literals;
        switch(result)
        {
        case ResolvePathResult::ErrorEmpty:
            return "error: path is empty"sv;
        case ResolvePathResult::ErrorInvalid:
            return "error: path is invalid"sv;
        case ResolvePathResult::ErrorMupltipleVariables:
            return "error: multiple variables in path"sv;
        case ResolvePathResult::ErrorVariablePlacement:
            return "error: placement of variables in path is invalid"sv;
        case ResolvePathResult::ErrorVariableConcatenated:
            return "error: variable in path is joined to more text"sv;
        case ResolvePathResult::Resolved:
            return "path valid"sv;
        case ResolvePathResult::ResolvedFound:
            return "path valid and found"sv;
        case ResolvePathResult::ResolvedNotFound:
            return "path valid and not found"sv;
        default:
            AFEX_ASSERT_FAIL("Unhandled resolve path result");
            return "UNHANDLED"sv;

        }
    }

    /*CORE_EXPORT*/ Filesystem::ResolvePathResult Filesystem::ResolvePath(const fs::path& pathIn, fs::path& outPath) const
    {
        outPath.clear();
        if(pathIn.empty())
        {
            return ResolvePathResult::ResolvedNotFound;
        }
        const fs::path firstPathPart = *pathIn.begin();

        const std::string pathInStr = pathIn.string();
        const std::string firstPathPartStr = firstPathPart.string();

        if (ContainsCaseInsensitive(firstPathPartStr, s_Assets))
        {
            if (StrCmp<Text::OrderingRules::CaseInsensitive>(firstPathPartStr, s_Assets) != 0)
            {
                return ResolvePathResult::ErrorVariableConcatenated;
            }
            AFEX_FATAL_CHECK_MSG(!m_AssetDirectories.empty(), "Filesystem was not created successfully. No asset directories were found.");

            // get all path parts after the asset directory
            fs::path endOfPath;
            for(fs::path::const_iterator pathIt = std::next(pathIn.begin()); pathIt != pathIn.end(); ++pathIt)
            {
                fs::path transformedPathPart;
                auto result = TryReplacePathSegmentWithVariables(
                    /*varMap=*/         m_VariableMap,
                    /*fullpath=*/       pathInStr,
                    /*strIn=*/          pathIt->string(),
                    /*pathOut=*/        transformedPathPart,
                    /*isFirstSegment=*/ false
                );
                switch (result)
                {
                case ResolvePathSegmentResult::ErrorWhitespacePlacement:
                    return ResolvePathResult::ErrorInvalid;
                case ResolvePathSegmentResult::ErrorVariableConcatenated:
                    return ResolvePathResult::ErrorVariableConcatenated;
                case ResolvePathSegmentResult::ErrorVariablePlacement:
                    return ResolvePathResult::ErrorVariablePlacement;
                }

                endOfPath = endOfPath / replaceEnvVariables(transformedPathPart.string());
            }

            // Go through the asset directories in reverse order to see if the requested path exists
            // If it does: return that path.
            // Otherwise: Resolve the path with the lowest ordered asset directory.
            for(auto it = m_AssetDirectories.crbegin(); it != m_AssetDirectories.crend(); it = std::next(it))
            {
                const fs::path& assetDirectory = *it;
                fs::path potentialResultPath = assetDirectory / endOfPath;

                potentialResultPath = fs::absolute(potentialResultPath);

                if(fs::exists(potentialResultPath))
                {
                    outPath = potentialResultPath;
                    return ResolvePathResult::ResolvedFound;
                }
            }

            outPath = fs::absolute(m_AssetDirectories[0] / endOfPath);
            return ResolvePathResult::ResolvedNotFound;
        }

        bool isFirstPathPart = true;
        for(const fs::path pathPart : pathIn)
        {
            fs::path transformedPathPart;
            auto result = TryReplacePathSegmentWithVariables(
                /*varMap=*/         m_VariableMap,
                /*fullpath=*/       pathInStr,
                /*strIn=*/          pathPart.string(),
                /*pathOut=*/        transformedPathPart,
                /*isFirstSegment=*/ isFirstPathPart
            );
            isFirstPathPart = false;
            switch(result)
            {
            case ResolvePathSegmentResult::ErrorWhitespacePlacement:
                return ResolvePathResult::ErrorInvalid;
            case ResolvePathSegmentResult::ErrorVariableConcatenated:
                return ResolvePathResult::ErrorVariableConcatenated;
            case ResolvePathSegmentResult::ErrorVariablePlacement:
                return ResolvePathResult::ErrorVariablePlacement;
            }

            outPath = outPath / replaceEnvVariables(transformedPathPart.string());
        }
        outPath = fs::absolute(outPath);

        return ResolvePathResult::Resolved;
    }
}