#pragma once

// Core
#include <Core/Core.export.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>

// System
#include <format>
#include <string>


#if defined(_DEBUG)
#define AFEX_ASSERT_ENABLED 1
#else
#define AFEX_ASSERT_ENABLED 0
#endif

#if AFEX_ASSERT_ENABLED
//////////////////////////////////////////////////////////////////////////
#define AFEX_ASSERT_FAIL(...)                                                                                   \
    do                                                                                                          \
    {                                                                                                           \
        static bool alwaysIgnore = false;                                                                       \
        if (!alwaysIgnore)                                                                                      \
        {                                                                                                       \
            if(Core::g_Logger)                                                                                  \
            {                                                                                                   \
                Core::g_Logger->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                    \
                    spdlog::level::critical,                                                                    \
                    std::format(                                                                                \
                        "Assert failed.\n"                                                                      \
                        "{}",                                                                                   \
                        std::format(__VA_ARGS__)));                                                             \
            }                                                                                                   \
            switch(Core::Internal::HandleAssertion("AFEX_ASSERT_FAIL",  std::format(__VA_ARGS__)))              \
            {                                                                                                   \
                case Core::Internal::DevAssertionResponse::BreakInDebugger: { AFEX_DEBUG_BREAK(); break;}       \
                case Core::Internal::DevAssertionResponse::IgnoreOnce:      { break;}                           \
                case Core::Internal::DevAssertionResponse::IgnoreAlways:    { alwaysIgnore = true; break;}      \
            }                                                                                                   \
        }                                                                                                       \
    }                                                                                                           \
    while(false);
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
            if(Core::g_Logger)                                                                                  \
            {                                                                                                   \
                Core::g_Logger->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                    \
                    spdlog::level::critical,                                                                    \
                    std::format(                                                                                \
                        "Assert({}) failed.\n"                                                                  \
                        "{}",                                                                                   \
                        #condition,                                                                             \
                        std::format(__VA_ARGS__)));                                                             \
            }                                                                                                   \
            switch(Core::Internal::HandleAssertion(#condition,  std::format(__VA_ARGS__)))                      \
            {                                                                                                   \
                case Core::Internal::DevAssertionResponse::BreakInDebugger: { AFEX_DEBUG_BREAK(); break;}       \
                case Core::Internal::DevAssertionResponse::IgnoreOnce:      { break;}                           \
                case Core::Internal::DevAssertionResponse::IgnoreAlways:    { alwaysIgnore = true; break;}      \
            }                                                                                                   \
        }                                                                                                       \
    }                                                                                                           \
    while(false);

// AFEX_FATAL_* acts as an assert when AFEX_ASSERT_ENABLED==1 and logs 
// fatally + terminates when asserts are disabled. In public builds the 
// messages are not stripped, so provide information to the user
// accordingly. The file and line are not logged when 
// AFEX_ASSERT_ENABLED==0, so provide the infromation neccesary to 
// understand which fatal error was encountered.
#define AFEX_FATAL_FAIL(...)                    AFEX_ASSERT_FAIL(__VA_ARGS__)
#define AFEX_FATAL_CHECK(condition)             AFEX_ASSERT(condition)
#define AFEX_FATAL_CHECK_MSG(condition, ...)    AFEX_ASSERT_MSG(condition, __VA_ARGS__)
#else // !AFEX_ASSERT_ENABLED
//////////////////////////////////////////////////////////////////////////
#define AFEX_ASSERT_FAIL(...)
#define AFEX_ASSERT_MSG(condition, ...)
#define AFEX_ASSERT_PUBLIC(condition, ...)

#define AFEX_FATAL_FAIL(...)                                                                \
    do                                                                                      \
    {                                                                                       \
        AFEX_LOG_FATAL(__VA_ARGS__);                                                        \
        /*TODO: Add a debug break where when we are certain this is an internal build.*/    \
        std::exit(EXIT_FAILURE);                                                            \
    }                                                                                       \
    while(false);

#define AFEX_FATAL_CHECK(condition)                                                             \
    do                                                                                          \
    {                                                                                           \
        if(!((condition)))                                                                      \
        {                                                                                       \
            AFEX_LOG_FATAL("AFEX_FATAL_CONDITION(" #condition ") failed");                      \
            /*TODO: Add a debug break where when we are certain this is an internal build.*/    \
            std::exit(EXIT_FAILURE);                                                            \
        }                                                                                       \
    }                                                                                           \
    while(false);

#define AFEX_FATAL_CHECK_MSG(condition, ...)                                                    \
    do                                                                                          \
    {                                                                                           \
        if(!((condition)))                                                                      \
        {                                                                                       \
            AFEX_LOG_FATAL(__VA_ARGS__);                                                        \
            /*TODO: Add a debug break where when we are certain this is an internal build.*/    \
            std::exit(EXIT_FAILURE);                                                            \
        }                                                                                       \
    }                                                                                           \
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