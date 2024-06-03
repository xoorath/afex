#include "AppLogging.h"

// Engine
#include <Core/Logging.h>

// External
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// System
#include <ctime>
#include <mutex>

namespace Donsol
{
    ////////////////////////////////////////////////////////////////////////// static
    static spdlog::sink_ptr CreateSink_MSVC()
    {
        const bool checkDebuggerPresent = true;
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::msvc_sink<std::mutex>>(checkDebuggerPresent);
        sink->set_level(spdlog::level::trace);
        return sink;
    }

    static spdlog::sink_ptr CreateSink_Stdout()
    {
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sink->set_level(spdlog::level::info);
        return sink;
    }

    static spdlog::sink_ptr CreateSink_File()
    {
        const spdlog::filename_t destination = SPDLOG_FILENAME_T("logs/afex.log");
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(destination);
        sink->set_level(spdlog::level::trace);
        return sink;
    }

    ////////////////////////////////////////////////////////////////////////// public
    void ConfigureLogging()
    {
        using namespace std::chrono_literals;

        Core::g_Logger = std::make_shared<spdlog::logger>
        (
            std::string("AppLogger"),
            spdlog::sinks_init_list
            ({
                CreateSink_MSVC(),
                CreateSink_Stdout(),
                CreateSink_File()
            })
        );
        Core::g_Logger->set_pattern("[%H:%M:%S.%e] [%t] [%L] %s(%#):  %v");
        Core::g_Logger->set_level(spdlog::level::trace);
        spdlog::register_logger(Core::g_Logger);
        
        std::time_t now;
        std::time(&now);
        char date_buf[64] = {0};
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        AFEX_LOG_INFO("({}) Logging has been configured."
        "============================================================", date_buf);

    }

    void ShutdownLogging()
    {
        spdlog::shutdown();
        Core::g_Logger.reset();
    }
}