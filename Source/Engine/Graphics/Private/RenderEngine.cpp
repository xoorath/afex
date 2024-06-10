#include <Graphics/RenderEngine.h>

// Graphics
#include "RenderEngineImpl.h"

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    ////////////////////////////////////////////////////////////////////////// Public

    /*GRAPHICS_EXPORT explicit*/ RenderEngine::RenderEngine(const RenderEngineArgs& args)
        : m_PIMPL(args)
    {
    }

    /*GRAPHICS_EXPORT*/ RenderEngine::~RenderEngine() = default;

    /*GRAPHICS_EXPORT*/ bool RenderEngine::IsValid() const
    {
        return m_PIMPL->IsValid();
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::SubmitFrame()
    {
        m_PIMPL->SubmitFrame();
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::WaitForRender()
    {
        m_PIMPL->WaitForRender();
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::Resize(uint32_t width, uint32_t height)
    {
        m_PIMPL->Resize(width, height);
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::SetDebugMode(DebugMode mode)
    {
        m_PIMPL->SetDebugMode(mode);
    }

    /*GRAPHICS_EXPORT*/ RenderEngine::RenderCallback& RenderEngine::OnRender()
    {
        return m_PIMPL->OnRender();
    }

    /*GRAPHICS_EXPORT*/ const bgfx::Stats* RenderEngine::GetStats() const
    {
        return m_PIMPL->GetStats();
    }
}