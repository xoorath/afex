#include "ApplicationImpl.h"

// Engine
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>
#include <Core/Filesystem/Paths.h>
#include <Core/Text.h>
#include <Platform/HMI/Cursor.h>
#include <Platform/HMI/Keyboard.h>
#include <Platform/HMI/ImguiInputProvider.h>
#include <Platform/WindowArgs.h>
#include <Graphics/DebugMode.h>
#include <Graphics/RenderEngineArgs.h>

// External
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// System
#include <ctime>
#include <mutex>

using namespace std::string_view_literals;

//////////////////////////////////////////////////////////////////////////
namespace Application
{
    ////////////////////////////////////////////////////////////////////////// Internal
    namespace
    {
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
            spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(destination, /*truncate=*/true);
            sink->set_level(spdlog::level::trace);
            return sink;
        }
    }

    ////////////////////////////////////////////////////////////////////////// Public
    ApplicationImpl::ApplicationImpl() = default;

    ApplicationImpl::~ApplicationImpl()
    {
        AFEX_ASSERT_MSG(m_ShutdownProcedure.size() == 0, "The application is meant to shutdown before being destroyed.");
    }
    
    bool ApplicationImpl::EarlyInit(int argc, const char* argv[])
    {
        AFEX_LOG_TRACE(__FUNCTION__ "()");

        if(Core::Paths::GlobalInit(argc, argv))
        {
            AddShutdownProcedure("Paths Global", [](){ Core::Paths::GlobalShutdown(); });
        }
        else
        {
            return false;
        }

        for(int i = 1; i < argc; ++i)
        {
            std::string_view arg = argv[i];

            if(Core::Text::StrCmp<Core::Text::OrderingRules::CaseInsensitive>(arg, "--config") == 0)
            {
                if(i == argc-1)
                {
                    break;
                }
                std::string_view value = argv[++i];
                std::filesystem::path configFilePath = value;
                if(std::filesystem::exists(configFilePath))
                {
                    Core::Config::ConfigFactory(configFilePath);
                    AddShutdownProcedure("release cached config"sv, 
                        [configFilePath]() 
                        {
                            std::string configName = configFilePath.filename().replace_extension().string();
                            Core::Config::ReleaseCachedConfig(configName);
                        });
                }
                else
                {
                    AFEX_LOG_ERROR("A config path was passed on the command line but it can't be found.\npath = ", configFilePath.string());
                }
            }
        }

        m_Config = Core::Config::ConfigFactory("afex"sv);
        if(m_Config.get() != nullptr)
        {
            AddShutdownProcedure("release engine config"sv, [this]() { m_Config.reset(); });
        }
        
        return true;
    }

    bool ApplicationImpl::Init()
    {
        AFEX_LOG_TRACE(__FUNCTION__ "()");
        ////////////////////////////////////////////////////////////////////////// Global Init
        if (Platform::Window::GlobalInit())
        {
            AddShutdownProcedure("Window Global"sv, &Platform::Window::GlobalShutdown);
        }
        else
        {
            return false;
        }

        ////////////////////////////////////////////////////////////////////////// Filesystem
        m_Filesystem.emplace(*m_Config.get());

        ////////////////////////////////////////////////////////////////////////// ImGui Context
        m_ImguiContext = ImGui::CreateContext();
        if(m_ImguiContext)
        {
            AddShutdownProcedure("ImGui Context", [this]()
            {
                ImGui::DestroyContext(m_ImguiContext); 
            });
        }
        else
        {
            return false;
        }

        ////////////////////////////////////////////////////////////////////////// Window
        const uint32_t width =      m_Config->GetSetting<uint32_t>("window.width", 1920);
        const uint32_t height =     m_Config->GetSetting<uint32_t>("window.height", 1080);
        const std::string title =   m_Config->GetSetting<std::string>("window.title", "afex");
        m_Config->Save();

        const Platform::WindowArgs windowArgs(title, m_ImguiContext, width, height);
        m_Window.emplace(windowArgs);
        if (m_Window->IsValid())
        {
            AddShutdownProcedure("Window",
                [&m_Window = m_Window]()
                {
                    m_Window.reset();
                });
        }
        else
        {
            return false;
        }

        ////////////////////////////////////////////////////////////////////////// Render Engine
        const Graphics::RenderEngineArgs renderArgs(m_Window->GetNativeWindowHandle(), m_ImguiContext, width, height);
        m_RenderEngine.emplace(renderArgs);
        if (m_RenderEngine->IsValid())
        {
            AddShutdownProcedure("Render Engine",
                [&m_RenderEngine = m_RenderEngine]()
                {
                    m_RenderEngine.reset();
                });
        }
        else
        {
            return false;
        }

        ////////////////////////////////////////////////////////////////////////// ImGui Renderer
        m_ImguiRenderer.emplace(m_ImguiContext, width, height);
        auto renderSubscription = m_RenderEngine->OnRender().Add(
            [&m_ImguiRenderer = m_ImguiRenderer]()
            {
                m_ImguiRenderer->Render();
            });
        AddShutdownProcedure("ImGui Renderer",
            [this, renderSubscription]()
            {
                m_RenderEngine->OnRender().Remove(renderSubscription);
                m_ImguiRenderer.reset();
            });

        //////////////////////////////////////////////////////////////////////////
        m_ImguiInputProvider.emplace(m_ImguiContext, m_Window->GetCursorMutable(), m_Window->GetKeyboardMutable());
        AddShutdownProcedure("ImGui Input Provider",
            [this]()
            {
                m_ImguiInputProvider.reset();
            });

        ////////////////////////////////////////////////////////////////////////// Resize Callbacks
        auto resizeSubscription = m_Window->OnResize().Add(
            [this](uint32_t width, uint32_t height)
            {
                // If the render resolution is set manually: do not resize automatically.
                // The application can always do that themselves.
                if(!m_RenderResolutionWidth.has_value() && !m_RenderResolutionHeight.has_value())
                {
                    m_RenderEngine->Resize(width, height);
                    m_ImguiRenderer->Resize(width, height);
                }
            });
        AddShutdownProcedure("Resize Subscription",
            [this, resizeSubscription]()
            {
                m_Window->OnResize().Remove(resizeSubscription);
            });

        return true;
    }

    bool ApplicationImpl::CloseRequested() const
    {
        return m_Window->CloseRequested();
    }

    void ApplicationImpl::PollEvents()
    {
        return m_Window->PollEvents();
    }

    void ApplicationImpl::BeginGUI()
    {
        m_ImguiRenderer->BeginFrame();
    }

    void ApplicationImpl::EndGUI()
    {
        m_ImguiRenderer->EndFrame();
    }

    void ApplicationImpl::SubmitFrame()
    {
        m_RenderEngine->SubmitFrame();
    }

    void ApplicationImpl::WaitForRenderer()
    {
        m_RenderEngine->WaitForRender();
    }

    void ApplicationImpl::AddShutdownProcedure(std::string_view debugName, std::function<void()> procedure)
    {
        m_ShutdownProcedure.push_back(std::make_tuple(std::string(debugName), procedure));
    }

    void ApplicationImpl::RunShutdownProcedure()
    {
        AFEX_LOG_TRACE(__FUNCTION__ "()");
        for (auto it = m_ShutdownProcedure.rbegin(); it != m_ShutdownProcedure.rend(); ++it)
        {
            AFEX_LOG_TRACE("Shutdown procedure: {}", (std::get<std::string>(*it)));
            (std::get<std::function<void()>>(*it))();
        }
        m_ShutdownProcedure.clear();
    }

    void ApplicationImpl::GetRenderResolution(uint32_t& outWidth, uint32_t& outHeight) const
    {
        if(m_RenderResolutionWidth.has_value() || m_RenderResolutionHeight.has_value())
        {
            if (m_RenderResolutionWidth.has_value())
            {
                outWidth = m_RenderResolutionWidth.value();
            }
            if (m_RenderResolutionHeight.has_value())
            {
                outHeight = m_RenderResolutionHeight.value();
            }
        }
        else
        {
            m_Window->GetSize(outWidth, outHeight);
        }
    }

    void ApplicationImpl::SetRenderResolution(uint32_t width, uint32_t height)
    {
        AFEX_ASSERT(width > 0);
        AFEX_ASSERT(height > 0);

        uint32_t windowWidth, windowHeight;
        m_Window->GetSize(windowWidth, windowHeight);

        if(width == windowWidth && height == windowHeight)
        {
            m_RenderResolutionWidth.reset();
            m_RenderResolutionHeight.reset();
        }
        else
        {
            m_RenderResolutionWidth = width;
            m_RenderResolutionHeight = height;
        }

        m_RenderEngine->Resize(width, height);
        m_ImguiRenderer->Resize(width, height);

        // Raw input is in window coordinates but we will be rendering at a different resolution.
        // Because of that we need to scale the input to ensure the render input lines up.

        const float inputScaleX = 
            static_cast<float>(width) / 
            static_cast<float>(windowWidth);
        const float inputScaleY = 
            static_cast<float>(height) /
            static_cast<float>(windowHeight);

        m_ImguiInputProvider->SetInputScale(inputScaleX, inputScaleY);
    }

    void ApplicationImpl::UnsetRenderResolution()
    {
        uint32_t windowWidth, windowHeight;
        m_Window->GetSize(windowWidth, windowHeight);

        m_RenderResolutionWidth.reset();
        m_RenderResolutionHeight.reset();

        m_RenderEngine->Resize(windowWidth, windowHeight);
        m_ImguiRenderer->Resize(windowWidth, windowHeight);

        m_ImguiInputProvider->SetInputScale(1.0f, 1.0f);
    }

    void ApplicationImpl::ConfigureLogging()
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
        char date_buf[64] = { 0 };
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        AFEX_LOG_INFO("({}) Logging has been configured."
            "============================================================", date_buf);

        AddShutdownProcedure("Logging", []()
        {
            std::time_t now;
            std::time(&now);
            char date_buf[64] = { 0 };
            std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

            AFEX_LOG_INFO("({}) App is shutting down."
                "============================================================", date_buf);
            spdlog::shutdown();
            Core::g_Logger.reset();
        });
    }
}