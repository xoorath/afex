#pragma once

// Engine
#include <Graphics/Graphics.export.h>

// System
#include <cstdint>

struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    class ImGuiRendererImpl;

    //////////////////////////////////////////////////////////////////////////
    class ImGuiRenderer
    {
    public:
        GRAPHICS_EXPORT ImGuiRenderer(ImGuiContext* context, uint32_t width, uint32_t height);
        GRAPHICS_EXPORT ImGuiRenderer(ImGuiRenderer&&) noexcept;
        GRAPHICS_EXPORT ImGuiRenderer& operator=(ImGuiRenderer&&) noexcept;
        GRAPHICS_EXPORT ~ImGuiRenderer();
        ImGuiRenderer()                                 = delete;
        ImGuiRenderer(const ImGuiRenderer&)             = delete;
        ImGuiRenderer& operator=(const ImGuiRenderer&)  = delete;

        GRAPHICS_EXPORT void BeginFrame();
        GRAPHICS_EXPORT void EndFrame();
        GRAPHICS_EXPORT void Render(); // <- Call from render thread

        GRAPHICS_EXPORT void Resize(uint32_t width, uint32_t height);

    private:
        ImGuiRendererImpl* m_PIMPL = nullptr;
    };
}