#pragma once

// Engine
#include <Core/Core.export.h>
#include <Core/Signal.h>
#include <Core/Config/ConfigValue.h>

// System
#include <cstdint>
#include <string_view>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    class Config;

    using SettingChangedCallback = Signal<
        void(
            Config& settings, 
            const std::string_view key,
            const ConfigValue& value)>;
}