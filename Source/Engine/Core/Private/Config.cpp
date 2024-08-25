#include <Core/Config/Config.h>

// Core
#include "ConfigImpl.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*CORE_EXPORT explicit*/ Config::Config(std::string_view collectionName)
        : m_PIMPL(collectionName)
    {
    }

    /*CORE_EXPORT explicit*/ Config::Config(const std::filesystem::path& collectionPath)
        : m_PIMPL(collectionPath)
    {
    }
    
    /*CORE_EXPORT*/ Config::~Config() = default;

    /*CORE_EXPORT*/ void Config::Save()
    {
        m_PIMPL->Save();
    }

    /*CORE_EXPORT*/ void Config::Load()
    {
        m_PIMPL->Load();
    }

#define AFEX_SETTINGS_IMPLEMENT_GET_SETTING(TYPE)                                                                               \
    template<>                                                                                                                  \
    void Config::SetSetting<TYPE>(std::string_view name, const TYPE& value)                                                     \
    {                                                                                                                           \
        m_PIMPL->SetSetting(name, ConfigValue(value));                                                                          \
    }                                                                                                                           \
    template<>                                                                                                                  \
    std::optional<TYPE> Config::GetSetting<TYPE>(std::string_view name)                                                         \
    {                                                                                                                           \
        auto found = m_PIMPL->GetSetting(name);                                                                                 \
        if (found.has_value())                                                                                                  \
        {                                                                                                                       \
            const bool correctType = std::holds_alternative<TYPE>(found.value());                                               \
            AFEX_ASSERT_MSG(correctType, "Config collection ({}) expects type " #TYPE " for variable ({})",                     \
                m_PIMPL->GetCollectionName(), name);                                                                            \
            if(correctType)                                                                                                     \
            {                                                                                                                   \
                return std::get<TYPE>(found.value());                                                                           \
            }                                                                                                                   \
        }                                                                                                                       \
        return std::nullopt;                                                                                                    \
    }                                                                                                                           \
    template<>                                                                                                                  \
    TYPE Config::GetSetting<TYPE>(std::string_view name, const TYPE& defaultValue)                                              \
    {                                                                                                                           \
        auto found = m_PIMPL->GetSetting(name, std::make_optional<ConfigValue>(defaultValue));                                  \
        if (found.has_value())                                                                                                  \
        {                                                                                                                       \
            const bool correctType = std::holds_alternative<TYPE>(found.value());                                               \
            AFEX_ASSERT_MSG(correctType, "Config collection ({}) expects type " #TYPE " for variable ({})",                     \
                m_PIMPL->GetCollectionName(), name);                                                                            \
            if(correctType)                                                                                                     \
            {                                                                                                                   \
                return std::get<TYPE>(found.value());                                                                           \
            }                                                                                                                   \
        }                                                                                                                       \
        return defaultValue;                                                                                                    \
    }

#define AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST(TYPE, ACTUAL_TYPE)                                                             \
    template<>                                                                                                                  \
    void Config::SetSetting<TYPE>(std::string_view name, const TYPE& value)                                                     \
    {                                                                                                                           \
        SetSetting<ACTUAL_TYPE>(name, static_cast<ACTUAL_TYPE>(value));                                                         \
    }                                                                                                                           \
    template<>                                                                                                                  \
    std::optional<TYPE> Config::GetSetting<TYPE>(std::string_view name)                                                         \
    {                                                                                                                           \
        auto result = GetSetting<ACTUAL_TYPE>(name);                                                                            \
        if(result.has_value())                                                                                                  \
        {                                                                                                                       \
            return static_cast<TYPE>(result.value());                                                                           \
        }                                                                                                                       \
        return std::nullopt;                                                                                                    \
    }                                                                                                                           \
    template<>                                                                                                                  \
    TYPE Config::GetSetting<TYPE>(std::string_view name, const TYPE& defaultValue)                                              \
    {                                                                                                                           \
        return static_cast<TYPE>(GetSetting<ACTUAL_TYPE>(name, static_cast<ACTUAL_TYPE>(defaultValue)));                        \
    }

#define AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST_VECTOR(TYPE, ACTUAL_TYPE)                                                      \
    template<>                                                                                                                  \
    void Config::SetSetting<std::vector<TYPE>>(std::string_view name, const std::vector<TYPE>& value)                           \
    {                                                                                                                           \
        std::vector<ACTUAL_TYPE> temp;                                                                                          \
        temp.reserve(value.size());                                                                                             \
        for(const auto& v : value)                                                                                              \
        {                                                                                                                       \
            temp.push_back(static_cast<ACTUAL_TYPE>(v));                                                                        \
        }                                                                                                                       \
        return SetSetting<std::vector<ACTUAL_TYPE>>(name, temp);                                                                \
    }                                                                                                                           \
    template<>                                                                                                                  \
    std::optional<std::vector<TYPE>> Config::GetSetting<std::vector<TYPE>>(std::string_view name)                               \
    {                                                                                                                           \
        auto result = GetSetting<std::vector<ACTUAL_TYPE>>(name);                                                               \
        if(result.has_value())                                                                                                  \
        {                                                                                                                       \
            auto value = result.value();                                                                                        \
            std::vector<TYPE> temp;                                                                                             \
            temp.reserve(value.size());                                                                                         \
            for(const auto& v : value)                                                                                          \
            {                                                                                                                   \
                temp.push_back(static_cast<TYPE>(v));                                                                           \
            }                                                                                                                   \
            return std::move(temp);                                                                                             \
        }                                                                                                                       \
        return std::nullopt;                                                                                                    \
    }                                                                                                                           \
    template<>                                                                                                                  \
    std::vector<TYPE> Config::GetSetting<std::vector<TYPE>>(std::string_view name, const std::vector<TYPE>& defaultValue)       \
    {                                                                                                                           \
        std::vector<ACTUAL_TYPE> tempDefaults;                                                                                  \
        tempDefaults.reserve(defaultValue.size());                                                                              \
        for(const auto& v : defaultValue)                                                                                       \
        {                                                                                                                       \
            tempDefaults.push_back(static_cast<ACTUAL_TYPE>(v));                                                                \
        }                                                                                                                       \
        auto result = GetSetting<std::vector<ACTUAL_TYPE>>(name, tempDefaults);                                                 \
        std::vector<TYPE> tempResult;                                                                                           \
        tempResult.reserve(result.size());                                                                                      \
        for(const auto& v : result)                                                                                             \
        {                                                                                                                       \
            tempResult.push_back(static_cast<TYPE>(v));                                                                         \
        }                                                                                                                       \
        return std::move(tempResult);                                                                                           \
    }

    // Directly supported types
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(int64_t)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(double)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(std::string)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(bool)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(std::chrono::system_clock::time_point)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(std::vector<int64_t>)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(std::vector<double>)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(std::vector<std::string>)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(std::vector<bool>)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING(std::vector<std::chrono::system_clock::time_point>)
    // Implemented as casts from one of the other types:
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST(uint64_t, int64_t)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST(uint32_t, int64_t)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST(int32_t, int64_t)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST(float, double)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST_VECTOR(uint64_t, int64_t)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST_VECTOR(uint32_t, int64_t)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST_VECTOR(int32_t, int64_t)
    AFEX_SETTINGS_IMPLEMENT_GET_SETTING_CAST_VECTOR(float, double)
}