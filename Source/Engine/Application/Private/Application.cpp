#include <Application/Application.h>

// Application
#include "ApplicationImpl.h"

// Engine
#include <Core/Logging.h>
#include <Platform/Window.h>

//////////////////////////////////////////////////////////////////////////
namespace Application
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*APPLICATION_EXPORT*/ Application::Application()
    {
    }

    /*APPLICATION_EXPORT*/ Application::~Application() = default;

    int32_t Application::Run()
    {
        // Important: just because an application may want to run init code 
        // before the engine is initialized: we very likely want m_PIMPL to 
        // be created before EarlyInit.
        m_PIMPL.emplace();
        if(!EarlyInit())
        {
            return -101;
        }

        if(!m_PIMPL->Init())
        {
            return -102;
        }

        if(!Init())
        {
            return -103;
        }

        while(!m_PIMPL->CloseRequested())
        {
            m_PIMPL->PollEvents();
            Render();
            m_PIMPL->BeginGUI();
            GUI();
            m_PIMPL->EndGUI();
            m_PIMPL->SubmitFrame();
            Update();
            m_PIMPL->WaitForRenderer();
        }

        return 0;
    }

    /*APPLICATION_EXPORT*/ void Application::GetRenderResolution(uint32_t& outWidth, uint32_t& outHeight) const
    {
        m_PIMPL->GetRenderResolution(outWidth, outHeight);
    }

    /*APPLICATION_EXPORT*/ void Application::SetRenderResolution(uint32_t width, uint32_t height)
    {
        m_PIMPL->SetRenderResolution(width, height);
    }

    ////////////////////////////////////////////////////////////////////////// Protected

    /*APPLICATION_EXPORT*/ void Application::AddShutdownProcedure(std::string_view debugName, std::function<void()> procedure)
    {
        return m_PIMPL->AddShutdownProcedure(debugName, procedure);
    }

    /*APPLICATION_EXPORT*/ const Graphics::RenderEngine& Application::GetRenderEngine() const
    {
        return m_PIMPL->GetRenderEngine();
    }

    /*APPLICATION_EXPORT*/ Graphics::RenderEngine& Application::GetRenderEngineMutable()
    {
        return m_PIMPL->GetRenderEngineMutable();
    }

    /*APPLICATION_EXPORT*/ const Platform::Window& Application::GetWindow() const
    {
        return m_PIMPL->GetWindow();
    }

    /*APPLICATION_EXPORT*/ Platform::Window& Application::GetWindowMutable()
    {
        return m_PIMPL->GetWindowMutable();
    }
}