#include "ApplicationImpl.h"

// Engine
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>
#include <Platform/HMI/Cursor.h>
#include <Platform/HMI/Keyboard.h>
#include <Platform/HMI/ImguiInputProvider.h>
#include <Platform/WindowArgs.h>
#include <Graphics/DebugMode.h>
#include <Graphics/RenderEngineArgs.h>

// External
#include <imgui.h>

//////////////////////////////////////////////////////////////////////////
namespace Application
{
    ////////////////////////////////////////////////////////////////////////// Public
    ApplicationImpl::~ApplicationImpl()
    {
        AFEX_LOG_TRACE(__FUNCTION__ "()");
        for (auto it = m_ShutdownProcedure.rbegin(); it != m_ShutdownProcedure.rend(); ++it)
        {
            AFEX_LOG_TRACE("Shutdown procedure: {}", (std::get<std::string>(*it)));
            (std::get<std::function<void()>>(*it))();
        }
    }
    
    bool ApplicationImpl::Init()
    {
        AFEX_LOG_TRACE(__FUNCTION__ "()");
        ////////////////////////////////////////////////////////////////////////// Global Init
        if (Platform::Window::GlobalInit())
        {
            AddShutdownProcedure("Window Global", &Platform::Window::GlobalShutdown);
        }
        else
        {
            return false;
        }

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
        const uint32_t width = m_Config.GetWidth();
        const uint32_t height = m_Config.GetHeight();
        const std::string_view title = m_Config.GetTitle();

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
                [&m_RenderEngine=m_RenderEngine]()
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
                width = static_cast<uint32_t>(static_cast<float>(width) / m_RenderScale);
                height = static_cast<uint32_t>(static_cast<float>(height) / m_RenderScale);
                m_RenderEngine->Resize(width, height);
                m_ImguiRenderer->Resize(width, height);
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

    float ApplicationImpl::GetRenderScale() const
    {
        return m_RenderScale;
    }

    void ApplicationImpl::SetRenderScale(float scale)
    {
        m_RenderScale = scale;

        uint32_t width, height;
        m_Window->GetSize(width, height);
        width = static_cast<uint32_t>(static_cast<float>(width) / m_RenderScale);
        height = static_cast<uint32_t>(static_cast<float>(height) / m_RenderScale);

        m_RenderEngine->Resize(width, height);
        m_ImguiRenderer->Resize(width, height);
        m_ImguiInputProvider->SetResolutionScale(m_RenderScale);
    }
}