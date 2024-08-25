#pragma once

// Engine
#include <Core/Core.export.h>
#include <Core/Config/ConfigValue.h>
#include <Core/PIMPL.h>

// System
#include <filesystem>
#include <functional>
#include <optional>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    class ConfigImpl;

    //////////////////////////////////////////////////////////////////////////
    // The collection name uniquely identifies these particular settings.
    // It's expected that the engine and application have it's own settings
    // that it creates once and uses from a single thread.
    // 
    // Gotchas:
    // 1. Creating multiple instances of the Config class with the same
    //  collection name is forbidden, or they must at least be mutually
    //  exclusive in their lifetimes.
    // 2. Config are currently backed by the toml file format. You can edit
    //  them by hand but should know that when the Save function is called:
    //  the file will be serialized. When that happens custom formatting and
    //  comments will be lost.
    class Config
    {
    public:
        // The collection name uniquely identifies this collection of 
        // configuration values. 
        //
        // On desktop platforms this refers to a filename next to the executable.
        // Game code should not be assumed that collections have any specific
        // backings such as toml, etc.
        CORE_EXPORT explicit Config(std::string_view collectionName);

        //////////////////////////////////////////////////////////////////////////
        // Creates a config from a toml file at a specific path.
        // 
        // IMPORTANT:
        // This override is discouraged for general use as it presumes a 
        // file-based backing for configs. It is used in development for command
        // line overrides of the afex config.
        CORE_EXPORT explicit Config(const std::filesystem::path& collectionPath);

        CORE_EXPORT ~Config(); /*=default*/
        
        Config()                                  = delete;
        Config(const Config&)                   = delete;
        Config& operator=(const Config&)        = delete;
        Config(Config&&) noexcept               = delete;
        Config& operator=(Config&&) noexcept    = delete;

        CORE_EXPORT void Save();
        CORE_EXPORT void Load();

        template<typename T>
        CORE_EXPORT void SetSetting(std::string_view name, const T& value);

        template<typename T>
        CORE_EXPORT std::optional<T> GetSetting(std::string_view name);
        template<typename T>
        CORE_EXPORT T GetSetting(std::string_view name, const T& defaultValue);

    private:
        Core::PIMPL<ConfigImpl, 152> m_PIMPL;
    };

    //////////////////////////////////////////////////////////////////////////
#define AFEX_SETTINGS_DECLARE_GET_SETTING(TYPE)                                                 \
    template<>                                                                                  \
    CORE_EXPORT void Config::SetSetting<TYPE>(std::string_view name, const TYPE& value);        \
    template<>                                                                                  \
    CORE_EXPORT std::optional<TYPE> Config::GetSetting<TYPE>(std::string_view name);            \
    template<>                                                                                  \
    CORE_EXPORT TYPE Config::GetSetting<TYPE>(std::string_view name, const TYPE& defaultValue);

    // Directly supported types:
    AFEX_SETTINGS_DECLARE_GET_SETTING(int64_t)
    AFEX_SETTINGS_DECLARE_GET_SETTING(double)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::string)
    AFEX_SETTINGS_DECLARE_GET_SETTING(bool)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::chrono::system_clock::time_point)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<int64_t>)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<double>)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<std::string>)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<bool>)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<std::chrono::system_clock::time_point>)
    // Implemented as casts from one of the other types:
    // Note about the vector types: they require extra memory and time to copy results and/or default params to the underlying type
    AFEX_SETTINGS_DECLARE_GET_SETTING(uint64_t)
    AFEX_SETTINGS_DECLARE_GET_SETTING(uint32_t)
    AFEX_SETTINGS_DECLARE_GET_SETTING(int32_t)
    AFEX_SETTINGS_DECLARE_GET_SETTING(float)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<uint64_t>)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<uint32_t>)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<int32_t>)
    AFEX_SETTINGS_DECLARE_GET_SETTING(std::vector<float>)

//#undef AFEX_SETTINGS_DECLARE_GET_SETTING
}