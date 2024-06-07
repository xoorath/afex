#include <Graphics/RenderEngine.h>

// Graphics
#include "bgfxCallbacks.h"
#include "Event.h"
#include "imguiRenderer.h"
#include "Logo.h"
#include "RenderEngineImpl.h"


// Engine
#include <Graphics/RenderEngineArgs.h>


// System
#include <atomic>
#include <limits>

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    /*GRAPHICS_EXPORT*/ RenderEngine::RenderEngine(const RenderEngineArgs& args)
    {
        auto* impl = new RenderEngineImpl(args);
        if (!impl->IsValid())
        {
            delete impl;
            impl = nullptr;
        }
        m_PIMPL = reinterpret_cast<void*>(impl);
    }

    /*GRAPHICS_EXPORT*/ RenderEngine::~RenderEngine()
    {
        Shutdown();
    }

    /*GRAPHICS_EXPORT*/ RenderEngine::RenderEngine(RenderEngine&& other) noexcept
        : m_PIMPL(other.m_PIMPL)
    {
        other.m_PIMPL = nullptr;
    }

    /*GRAPHICS_EXPORT*/ RenderEngine& RenderEngine::operator=(RenderEngine&& other) noexcept
    {
        m_PIMPL = other.m_PIMPL;
        other.m_PIMPL = nullptr;
        return *this;
    }

    /*GRAPHICS_EXPORT*/ bool RenderEngine::IsValid() const
    {
        // Note: If initialization of the RenderEngineImpl fails, it will be null after construction
        return m_PIMPL != nullptr;
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::Shutdown()
    {
        delete reinterpret_cast<RenderEngineImpl*>(m_PIMPL);
        m_PIMPL = nullptr;
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::BeginFrame() const
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->BeginFrame();
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::EndFrame() const
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->EndFrame();
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::Resize(uint32_t width, uint32_t height)
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->Resize(width, height);
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::SetDebugMode(DebugMode mode)
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->SetDebugMode(mode);
    }
}