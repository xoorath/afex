#include <Graphics/RenderEngine.h>

// Graphics
#include "RenderEngineImpl.h"

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
        delete reinterpret_cast<RenderEngineImpl*>(m_PIMPL);
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

    /*GRAPHICS_EXPORT*/ void RenderEngine::SubmitFrame()
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->SubmitFrame();
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::WaitForRender()
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->WaitForRender();
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::Resize(uint32_t width, uint32_t height)
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->Resize(width, height);
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::SetDebugMode(DebugMode mode)
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->SetDebugMode(mode);
    }

    /*GRAPHICS_EXPORT*/ RenderEngine::RenderCallback& RenderEngine::OnRender()
    {
        return reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->OnRender();
    }
}