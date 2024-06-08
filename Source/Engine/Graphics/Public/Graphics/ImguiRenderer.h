#pragma once

// Engine
#include <Graphics/Graphics.export.h>

// System
#include <cstdint>

struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
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

        GRAPHICS_EXPORT void BeginFrame() const;
        GRAPHICS_EXPORT void EndFrame() const;
        GRAPHICS_EXPORT void Render() const; // <- Call from render thread

        GRAPHICS_EXPORT void Resize(uint32_t width, uint32_t height);

    private:
        void* m_PIMPL = nullptr;
    };
}