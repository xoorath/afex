#pragma once

// Engine
#include <Graphics/Graphics.export.h>

// System
#include <cstdint>

namespace Graphics
{
    enum class DebugMode : uint8_t;
    class RenderEngineArgs;

    // The RenderEngine manages our interactions with the rendering backend (bgfx)
    // and sets up the render thread.
    class RenderEngine
    {
    public:
        GRAPHICS_EXPORT explicit RenderEngine(const RenderEngineArgs& args);
        GRAPHICS_EXPORT ~RenderEngine();
        GRAPHICS_EXPORT RenderEngine(RenderEngine&& other) noexcept;
        GRAPHICS_EXPORT RenderEngine& operator=(RenderEngine&& other) noexcept;
        RenderEngine(const RenderEngine&)               = delete;
        RenderEngine& operator=(const RenderEngine&)    = delete;

        // If the render engine is not valid after construction: it cannot be used and should be destroyed.
        GRAPHICS_EXPORT bool IsValid() const;

        // Manually shuts down the render engine. This will happen at destruction, 
        // but is exposed so it can optionally be done manually.
        GRAPHICS_EXPORT void Shutdown();

        // Only one thread is permitted to submit render calls.
        // Whichever thread that is should call BeginFrame/EndFrame when rendering is starting or ending the frame.
        GRAPHICS_EXPORT void BeginFrame() const;
        GRAPHICS_EXPORT void EndFrame() const;

        // Resizes the render target (does not take effect immediately)
        GRAPHICS_EXPORT void Resize(uint32_t width, uint32_t height);

        // Sets the bgfx debug mode
        GRAPHICS_EXPORT void SetDebugMode(DebugMode mode);

    private:
        void* m_PIMPL = nullptr;
    };
}