#pragma once

// Engine
#include <Core/Core.export.h>

// External
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

// System
#include <memory.h>

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
    #define AFEX_LOG_TRACE(...) SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::trace, __VA_ARGS__)
    #define AFEX_LOG_INFO(...) SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::info, __VA_ARGS__)
    #define AFEX_LOG_ERROR(...) SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::err, __VA_ARGS__)
#else
    #define AFEX_LOG_TRACE(...)
    #define AFEX_LOG_INFO(...)
    #define AFEX_LOG_ERROR(...) SPDLOG_LOGGER_CALL(::Core::g_Logger, spdlog::level::err, __VA_ARGS__)
#endif
