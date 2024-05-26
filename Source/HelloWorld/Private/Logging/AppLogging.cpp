#include "AppLogging.h"

// Engine
#include <Core/Logging.h>

// External
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

// System
#include <chrono>
#include <mutex>
#include <vector>

namespace HelloWorld
{
    spdlog::sink_ptr CreateSink_MSVC()
    {
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::msvc_sink<std::mutex>>
        (
            /*check_debugger_present=*/ true
        );
        sink->set_level(/*log_level=*/spdlog::level::debug);
        return sink;
    }

    spdlog::sink_ptr CreateSink_File()
    {
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>
        (
            /*filename=*/ SPDLOG_FILENAME_T("logs/afex.log")
        );
        sink->set_level(/*log_level=*/spdlog::level::info);
        return sink;
    }

    void ConfigureLogging()
    {
        using namespace std::chrono_literals;
        spdlog::flush_every(3s);

        Core::g_Logger = std::make_shared<spdlog::logger>
        (
            /*name=*/ std::string("AppLogger"),
            /*sinks=*/ 
            spdlog::sinks_init_list({
                CreateSink_MSVC(),
                CreateSink_File()
            })
        );

        Core::g_Logger->set_pattern(/*pattern=*/"[%H:%M:%S.%e] [%t] [%L] %s(%#):  %v");

        AFEX_LOG_INFO("Logging has been configured.");
    }
}