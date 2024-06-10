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
            (*it)();
        }
    }
    
    bool ApplicationImpl::Init()
    {
        AFEX_LOG_TRACE(__FUNCTION__ "()");
        if (Platform::Window::GlobalInit())
        {
            AddShutdownProcedure(&Platform::Window::GlobalShutdown);
        }
        else
        {
            return false;
        }

        m_ImguiContext = ImGui::CreateContext();
        if(m_ImguiContext)
        {
            AddShutdownProcedure([this]()
            {
                ImGui::DestroyContext(m_ImguiContext); 
            });
        }
        else
        {
            return false;
        }

        const uint32_t width = m_Config.GetWidth();
        const uint32_t height = m_Config.GetHeight();
        const std::string_view title = m_Config.GetTitle();

        const Platform::WindowArgs windowArgs(title, m_ImguiContext, width, height);
        m_Window.emplace(windowArgs);
        if (m_Window->IsValid())
        {
            AddShutdownProcedure([this]() { m_Window.reset(); });
        }
        else
        {
            return false;
        }

        const Graphics::RenderEngineArgs renderArgs(m_Window->GetNativeWindowHandle(), m_ImguiContext, width, height);
        m_RenderEngine.emplace(renderArgs);
        if (m_RenderEngine->IsValid())
        {
            AddShutdownProcedure([this]() { m_RenderEngine.reset(); });
        }
        else
        {
            return false;
        }

        m_ImguiRenderer.emplace(m_ImguiContext, width, height);
        auto renderSubscription = m_RenderEngine->OnRender().Add(
            [this]()
            {
                m_ImguiRenderer->Render();
            });
        AddShutdownProcedure(
            [this, renderSubscription]()
            {
                m_RenderEngine->OnRender().Remove(renderSubscription);
                m_ImguiRenderer.reset();
            });

        m_Window->OnResize().Add(
            [this](uint32_t width, uint32_t height)
            {
                m_RenderEngine->Resize(width, height);
                m_ImguiRenderer->Resize(width, height);
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

    void ApplicationImpl::AddShutdownProcedure(std::function<void()> procedure)
    {
        m_ShutdownProcedure.push_back(procedure);
    }
}