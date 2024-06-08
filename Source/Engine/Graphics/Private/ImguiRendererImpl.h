#pragma once

// System
#include <atomic>
#include <cstdint>

// External
#include <bgfx/bgfx.h>

//////////////////////////////////////////////////////////////////////////

struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    ////////////////////////////////////////////////////////////////////////// internal
    class ImGuiRendererImpl
    {
    public:
        ImGuiRendererImpl(ImGuiContext* context, uint32_t width, uint32_t height);
        ~ImGuiRendererImpl();

        void BeginFrame();
        void EndFrame();
        void Render();

        void Resize(uint32_t width, uint32_t height);
    private:
        bgfx::VertexLayout  m_VertexLayout;
        bgfx::TextureHandle m_FontTexture = bgfx::kInvalidHandle;
        bgfx::UniformHandle m_FontUniform = bgfx::kInvalidHandle;
        bgfx::ProgramHandle m_Program = bgfx::kInvalidHandle;

        // Storing width and height together is a little "cute" but it lets us perform the set atomically.
        std::atomic_uint64_t m_WidthHeight = 0;
        uint32_t m_PreviousWidth = 0;
        uint32_t m_PreviousHeight = 0;

        ImGuiContext* m_ImguiContext;
    };
}