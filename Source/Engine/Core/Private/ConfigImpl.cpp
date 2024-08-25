#include "ConfigImpl.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>
#include <Core/Paths.h>
#include <Core/Text.h>

// System
#include <filesystem>
#include <sstream>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    ////////////////////////////////////////////////////////////////////////// Internal
    std::string timePointToString(const std::chrono::system_clock::time_point& timePoint) {
        // Convert time_point to string (for example, as ISO8601 format)
        std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
        std::tm tm = *std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    ////////////////////////////////////////////////////////////////////////// Public

    /*explicit*/ ConfigImpl::ConfigImpl(std::string_view collectionName)
        : m_CollectionName(collectionName)
    {
        const std::string invalidChars = "<>:\"/\\|?*";
        AFEX_ASSERT_MSG(m_CollectionName.find_first_of(invalidChars) == std::string::npos,
            "Config collection name is invalid: {}", m_CollectionName);

        namespace fs = std::filesystem;

        // We can't use the Core::Filesystem module here as the filesystem requires
        // this config module to be setup.
        // One option for the future is: we could potentially take an optional filesystem
        // reference and use it if it exists
        m_FilePath = Core::Paths::ApplicationDirectory() / (std::string(collectionName) + ".toml");

        Load();
    }

    ConfigImpl::ConfigImpl(const std::filesystem::path& collectionPath)
    {
        AFEX_ASSERT_MSG(!collectionPath.empty(), "A config was specified an empty path\npath = {}", collectionPath.string());
        AFEX_ASSERT_MSG(collectionPath.has_filename(), "A config was specified with no filename\npath = {}", collectionPath.string());
        
        using namespace Core::Text;

#if AFEX_LOG_WARNING_ENABLED
        if(!collectionPath.has_extension() 
            || StrCmp<OrderingRules::CaseInsensitive>(collectionPath.extension().string(), ".toml") != 0)
        {
            AFEX_LOG_WARNING("A config file was specified without the toml extension\npath = {}", collectionPath.string());
        }
#endif

        m_CollectionName = collectionPath.filename().replace_extension().string();
        m_FilePath = collectionPath;

        Load();
    }

    void ConfigImpl::Load()
    {
        try
        {
            m_ConfigTable = toml::parse_file(m_FilePath.string());
        }
        catch (const toml::v3::ex::parse_error& ex)
        {
            (void)ex;
            AFEX_LOG_ERROR("{}: {}", ex.description(), m_FilePath.string());
        }
    }

    void ConfigImpl::Save()
    {
        const std::string filePathStr = m_FilePath.string();

        std::ofstream file(filePathStr);
        if (file.is_open())
        {
            file << m_ConfigTable;
            file.close();
            AFEX_LOG_TRACE("Config file saved: {}", filePathStr);
        }
        else
        {
            AFEX_LOG_ERROR("Could not save settings file: {}", filePathStr);
        }
    }

    std::optional<ConfigValue> ConfigImpl::GetSetting(std::string_view name, const std::optional<ConfigValue>& defaultValue)
    {
        auto table = FindTable(name, defaultValue.has_value());
        if(table == nullptr)
        {
            return std::nullopt;
        }

        size_t lastDotPos = name.find_last_of('.');
        const std::string_view varName = (lastDotPos == std::string::npos) ? name : name.substr(lastDotPos+1);

        auto found = table->find(varName);
        if (found != table->end())
        {
            return FromToml(found->second);
        }
        else
        {
            if (defaultValue.has_value())
            {
                auto converted = ToToml(defaultValue.value());
                table->insert_or_assign(varName, *converted.get());
                return defaultValue.value();
            }
            return std::nullopt;
        }
    }

    void ConfigImpl::SetSetting(std::string_view name, const ConfigValue& value)
    {
        auto table = FindTable(name, true);
        AFEX_ASSERT_MSG(table != nullptr, "Could not create a settings value for the setting: {}", name);
        if(table != nullptr)
        {
            size_t lastDotPos = name.find_last_of('.');
            if(lastDotPos == std::string::npos)
            {
                lastDotPos = 0;
            }
            else
            {
                lastDotPos += 1;
            }
            const std::string_view varName = name.substr(lastDotPos);

            auto converted = ToToml(value);
            table->insert_or_assign(varName, *converted.get());
        }
    }

    ////////////////////////////////////////////////////////////////////////// Private

    std::unique_ptr<toml::node> ConfigImpl::ToToml(const ConfigValue& value)
    {
        return std::visit(
            [](auto&& arg) -> std::unique_ptr<toml::node>
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int64_t>)
                {
                    return std::make_unique<toml::value<int64_t>>(arg);
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    return std::make_unique<toml::value<double>>(arg);
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    return std::make_unique<toml::value<std::string>>(arg);
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    return std::make_unique<toml::value<bool>>(arg);
                }
                else if constexpr (std::is_same_v<T, std::chrono::system_clock::time_point>)
                {
                    // Convert time_point to string
                    auto timeString = timePointToString(arg);
                    return std::make_unique<toml::value<std::string>>(timeString);
                }
                else if constexpr (std::is_same_v<T, std::vector<int64_t>>)
                {
                    toml::array arr;
                    for (const auto& elem : arg)
                    {
                        arr.push_back(toml::value<int64_t>(elem));
                    }
                    return std::make_unique<toml::array>(std::move(arr));
                }
                else if constexpr (std::is_same_v<T, std::vector<double>>)
                {
                    toml::array arr;
                    for (const auto& elem : arg)
                    {
                        arr.push_back(toml::value<double>(elem));
                    }
                    return std::make_unique<toml::array>(std::move(arr));
                }
                else if constexpr (std::is_same_v<T, std::vector<std::string>>)
                {
                    toml::array arr;
                    for (const auto& elem : arg)
                    {
                        arr.push_back(toml::value<std::string>(elem));
                    }
                    return std::make_unique<toml::array>(std::move(arr));
                }
                else if constexpr (std::is_same_v<T, std::vector<bool>>)
                {
                    toml::array arr;
                    for (const auto& elem : arg)
                    {
                        arr.push_back(toml::value<bool>(elem));
                    }
                    return std::make_unique<toml::array>(std::move(arr));
                }
                else if constexpr (std::is_same_v<T, std::vector<std::chrono::system_clock::time_point>>)
                {
                    toml::array arr;
                    for (const auto& elem : arg)
                    {
                        auto timeString = timePointToString(elem);
                        arr.push_back(toml::value<std::string>(timeString));
                    }
                    return std::make_unique<toml::array>(std::move(arr));
                }
                else
                {
                    return nullptr;
                }
            }, value);
    }

    ConfigValue ConfigImpl::FromToml(const toml::node& node)
    {
        if (const auto* intVal = node.as_integer())
        {
            return intVal->get();
        }
        else if (const auto* floatVal = node.as_floating_point())
        {
            return floatVal->get();
        }
        else if (const auto* stringVal = node.as_string())
        {
            return stringVal->get();
        }
        else if (const auto* boolVal = node.as_boolean())
        {
            return boolVal->get();
        }
        else if (const auto* arr = node.as_array())
        {
            if (arr->empty()) return {};

            const toml::node& firstElem = arr->front();
            if (firstElem.is_integer())
            {
                std::vector<int64_t> vec;
                for (const auto& elem : *arr)
                {
                    vec.push_back(elem.as_integer()->get());
                }
                return vec;
            }
            else if (firstElem.is_floating_point())
            {
                std::vector<double> vec;
                for (const auto& elem : *arr)
                {
                    vec.push_back(elem.as_floating_point()->get());
                }
                return vec;
            }
            else if (firstElem.is_string())
            {
                std::vector<std::string> vec;
                for (const auto& elem : *arr)
                {
                    vec.push_back(elem.as_string()->get());
                }
                return vec;
            }
            else if (firstElem.is_boolean())
            {
                std::vector<bool> vec;
                for (const auto& elem : *arr) {
                    vec.push_back(elem.as_boolean()->get());
                }
                return vec;
            }
        }
        throw std::logic_error("All supported toml types should be represented above.");
    }

    toml::table* ConfigImpl::FindTable(std::string_view path, bool createMissingPaths)
    {
        auto dotPos = path.find('.');
        if (dotPos == std::string::npos)
        {
            // return the root table
            return &m_ConfigTable;
        }
        else 
        {
            const size_t dotCount = std::count(path.begin(), path.end(), '.');
            std::vector<std::string_view> subPaths;
            subPaths.reserve(dotCount);
            
            size_t dotStart = 0;
            while(dotPos != std::string::npos)
            {
                subPaths.push_back(path.substr(dotStart, dotPos - dotStart));
                dotStart = dotPos+1;
                dotPos = path.find('.', dotPos+1);
            }

            toml::table* currentTable = &m_ConfigTable;

            // -1 so we stop before getting to the variable name. We're just looking for the table for now.
            for(size_t i = 0; i < subPaths.size(); ++i)
            {
                const auto& subPath = subPaths[i];
                auto found = currentTable->find(subPath);
                if(found == currentTable->end())
                {
                    if(createMissingPaths)
                    {
                        auto result = currentTable->insert_or_assign(subPath, toml::table{});
                        found = result.first;
                        if (found == currentTable->end())
                        {
                            // We couldn't create a new path
                            return nullptr;
                        }
                        currentTable = found->second.as_table();
                    }
                    else
                    {
                        // The caller doesn't want to create new paths
                        return nullptr;
                    }
                }
                else
                {
                    currentTable = found->second.as_table();
                }
            }
            return currentTable;
        }
    }

}