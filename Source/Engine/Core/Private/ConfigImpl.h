#pragma once

// Engine
#include <Core/Config/ConfigValue.h>

// System
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string_view>
#include <unordered_map>

// External
#include <toml.hpp>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    class Config;

    //////////////////////////////////////////////////////////////////////////
    class ConfigImpl
    {
    public:
        static std::shared_ptr<Config> ConfigFactory(std::string_view collectionName);

        static std::shared_ptr<Config> ConfigFactory(const std::filesystem::path& collectionPath);

        static void ReleaseCachedConfig(std::string_view collectionName);

        // The collection name will ultimately become a file name in reality,
        // however the application should not make assumptions about the
        // filesystem or underlying structure of settings outside of the 
        // few constraints we have such as the collection name being a valid
        // filename on all supported systems.
        explicit ConfigImpl(std::string_view collectionName);

        // This override should not be used by the application. 
        // It's intended for development-time overrides of the afex config
        explicit ConfigImpl(const std::filesystem::path& collectionPath);

        ConfigImpl() = default;
        ConfigImpl(const ConfigImpl&) = default;
        ConfigImpl(ConfigImpl&&) noexcept = default;
        ConfigImpl& operator=(const ConfigImpl&) = default;
        ConfigImpl& operator=(ConfigImpl&&) noexcept = default;
        ~ConfigImpl() = default;

        const std::string_view GetCollectionName() const { return m_CollectionName; }

        void Load();
        void Save();

        std::optional<ConfigValue> GetSetting(
            std::string_view name,
            const std::optional<ConfigValue>& defaultValue = std::nullopt);

        void SetSetting(std::string_view name, const ConfigValue& value);

    private:
        static std::unique_ptr<toml::node> ToToml(const ConfigValue& value);
        static ConfigValue FromToml(const toml::node& node);
        // a variable like "alpha.a" requires the alpha table and a variable "b" needs the root table
        // This function gets the appropriate table for a given name.
        toml::table* FindTable(std::string_view varName, bool createMissingPaths);

        static std::map<std::string, std::shared_ptr<Config>> s_ConfigCache;
        static std::mutex s_FactoryMutex;

        std::string m_CollectionName;
        std::filesystem::path m_FilePath;
        toml::table m_ConfigTable;
    };
}