#include <Graphics/ImguiRenderer.h>

// Graphics
#include "ImguiRendererImpl.h"

// Engine
#include <Graphics/ViewId.h>



//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    ////////////////////////////////////////////////////////////////////////// public
    /*GRAPHICS_EXPORT*/ ImGuiRenderer::ImGuiRenderer(ImGuiContext* context, uint32_t width, uint32_t height)
        : m_PIMPL(reinterpret_cast<void*>(new ImGuiRendererImpl(context, width, height)))
    {
    }

    /*GRAPHICS_EXPORT*/ ImGuiRenderer::ImGuiRenderer(ImGuiRenderer&& other) noexcept
        : m_PIMPL(other.m_PIMPL)
    {
        other.m_PIMPL = nullptr;
    }

    /*GRAPHICS_EXPORT*/ ImGuiRenderer& ImGuiRenderer::operator=(ImGuiRenderer&& other) noexcept
    {
        m_PIMPL = other.m_PIMPL;
        other.m_PIMPL = nullptr;
        return *this;
    }

    /*GRAPHICS_EXPORT*/ ImGuiRenderer::~ImGuiRenderer()
    {
        delete reinterpret_cast<ImGuiRendererImpl*>(m_PIMPL);
    }


    /*GRAPHICS_EXPORT*/ void ImGuiRenderer::BeginFrame()
    {
        reinterpret_cast<ImGuiRendererImpl*>(m_PIMPL)->BeginFrame();
    }

    /*GRAPHICS_EXPORT*/ void ImGuiRenderer::EndFrame()
    {
        reinterpret_cast<ImGuiRendererImpl*>(m_PIMPL)->EndFrame();
    }

    /*GRAPHICS_EXPORT*/ void ImGuiRenderer::Render()
    {
        reinterpret_cast<ImGuiRendererImpl*>(m_PIMPL)->Render();
    }

    /*GRAPHICS_EXPORT*/ void ImGuiRenderer::Resize(uint32_t width, uint32_t height)
    {
        reinterpret_cast<ImGuiRendererImpl*>(m_PIMPL)->Resize(width, height);
    }
}