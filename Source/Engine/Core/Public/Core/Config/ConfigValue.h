#pragma once

// Engine
#include <Core/Core.export.h>

// System
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    //////////////////////////////////////////////////////////////////////////
    using ConfigValue =
        std::variant<
            int64_t,
            double,
            std::string,
            bool,
            std::chrono::system_clock::time_point,
            std::vector<int64_t>,
            std::vector<double>,
            std::vector<std::string>,
            std::vector<bool>,
            std::vector<std::chrono::system_clock::time_point>
        >;
}