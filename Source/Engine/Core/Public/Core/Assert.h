#pragma once

// Core
#include <Core/Core.export.h>
#include <Core/CommonMacros.h>

// System
#include <format>
#include <string>

#if defined(_DEBUG)
//////////////////////////////////////////////////////////////////////////
#define AFEX_ASSERT_MSG(condition, ...)                                                                         \
    do                                                                                                          \
    {                                                                                                           \
        static bool alwaysIgnore = false;                                                                       \
        _Pragma("warning(push)")                                                                                \
        _Pragma("warning(disable: 4127)") /* C4127 conditional expression is constant */                        \
        /* check condition first. The consistent behavior should be that condition is always executed.*/        \
        if (!((condition)) && !alwaysIgnore)                                                                    \
        _Pragma("warning(pop)")                                                                                 \
        {                                                                                                       \
            Core::g_Logger->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::err,    \
                std::format(                                                                                    \
                    "Assert({}) failed.\n"                                                                      \
                    "{}",                                                                                       \
                    #condition,                                                                                 \
                    std::format(__VA_ARGS__)));                                                                 \
            switch(Core::Internal::HandleAssertion(#condition,  std::format(__VA_ARGS__)))                      \
            {                                                                                                   \
                case Core::Internal::DevAssertionResponse::BreakInDebugger: { AFEX_DEBUG_BREAK(); break;}       \
                case Core::Internal::DevAssertionResponse::IgnoreOnce:      { break;}                           \
                case Core::Internal::DevAssertionResponse::IgnoreAlways:    { alwaysIgnore = true; break;}      \
            }                                                                                                   \
        }                                                                                                       \
    }                                                                                                           \
    while(false);
#else
//////////////////////////////////////////////////////////////////////////
#define AFEX_ASSERT_MSG(condition, ...)                                                                             \
    do                                                                                                          \
    {                                                                                                           \
        AFEX_UNUSED((condition));                                                                               \
    }                                                                                                           \
    while(false);
#endif

#define AFEX_ASSERT(condition) AFEX_ASSERT_MSG(condition, #condition)

//////////////////////////////////////////////////////////////////////////
namespace Core::Internal
{
    enum class DevAssertionResponse
    {
        BreakInDebugger,
        IgnoreOnce,
        IgnoreAlways
    };

    // Prompts the user to break into the debugger, ignore once or ignore always
    // Returns the result of that prompt
    CORE_EXPORT DevAssertionResponse HandleAssertion(const std::string& conditionText, const std::string& message);
}