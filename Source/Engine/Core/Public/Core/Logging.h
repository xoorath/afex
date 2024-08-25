#pragma once

// Engine
#include <Core/Core.export.h>
#include <Core/CommonMacros.h>

// External
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

// System
#include <memory.h>
#include <format>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    // IMPORTANT: Each application is required to configure this logger
    // before the Core library can be used. It's expected that this logger
    // is a thread safe variant. See: https://github.com/gabime/spdlog
    CORE_EXPORT extern std::shared_ptr<spdlog::logger> g_Logger;
}

//////////////////////////////////////////////////////////////////////////
// These macros are to be used by all modules in the engine and can be 
// used by any applications using the engine.
#if defined(_DEBUG)
    #define AFEX_LOG_TRACE(...)         SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::trace, __VA_ARGS__)
    #define AFEX_LOG_TRACE_ENABLED      1
    #define AFEX_LOG_INFO(...)          SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::info, __VA_ARGS__)
    #define AFEX_LOG_INFO_ENABLED       1
    #define AFEX_LOG_WARNING(...)       SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::warn, __VA_ARGS__)
    #define AFEX_LOG_WARNING_ENABLED    1
    #define AFEX_LOG_ERROR(...)         SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::err, __VA_ARGS__)
    #define AFEX_LOG_ERROR_ENABLED      1
    // Note: fatal logs are not stripped from release builds. Do not log anything too sensitive for public eyes.
    #define AFEX_LOG_FATAL(...)         SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::critical, __VA_ARGS__)
    #define AFEX_LOG_FATAL_ENABLED      1
#else
    // Todo: consider adding telemetry for the number of warnings and errors we hit with some unique ID representing
    // the source of that/those warnings and errors. That may give us better visibility into problems in live builds
    // without leaking much log information in the build.
    #define AFEX_LOG_TRACE(...)
    #define AFEX_LOG_TRACE_ENABLED      0
    #define AFEX_LOG_INFO(...)
    #define AFEX_LOG_INFO_ENABLED       0
    #define AFEX_LOG_WARNING(...)
    #define AFEX_LOG_WARNING_ENABLED    0
    #define AFEX_LOG_ERROR(...)
    #define AFEX_LOG_ERROR_ENABLED      0
    #define AFEX_LOG_FATAL(...)     SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::critical, __VA_ARGS__)
    #define AFEX_LOG_FATAL_ENABLED      1
#endif
