#pragma once

// Engine
#include <Core/PIMPL.h>
#include <Core/Signal.h>
#include <Graphics/Graphics.export.h>

// System
#include <cstdint>

//////////////////////////////////////////////////////////////////////////

namespace bgfx
{
    struct Stats;
}

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    enum class DebugMode : uint8_t;
    class RenderEngineArgs;
    class RenderEngineImpl;

    //////////////////////////////////////////////////////////////////////////
    // The RenderEngine manages our interactions with the rendering backend (bgfx)
    // and sets up the render thread.
    class RenderEngine
    {
    public:
        using RenderCallback = Core::Signal<void()>;

        GRAPHICS_EXPORT explicit RenderEngine(const RenderEngineArgs& args);
        GRAPHICS_EXPORT ~RenderEngine();                        /*=default*/
        RenderEngine(RenderEngine&& other) noexcept             = delete;
        RenderEngine& operator=(RenderEngine&& other) noexcept  = delete;
        RenderEngine(const RenderEngine&)                       = delete;
        RenderEngine& operator=(const RenderEngine&)            = delete;

        // If the render engine is not valid after construction it cannot be used and should be destroyed.
        GRAPHICS_EXPORT bool IsValid() const;

        // Only one thread is permitted to submit render calls.
        // Whichever thread that is should call SubmitFrame when rendering is starting or ending the frame.
        GRAPHICS_EXPORT void SubmitFrame();
        GRAPHICS_EXPORT void WaitForRender();

        // Resizes the render target (does not take effect immediately)
        GRAPHICS_EXPORT void Resize(uint32_t width, uint32_t height);

        // Sets the bgfx debug mode
        GRAPHICS_EXPORT void SetDebugMode(DebugMode mode);

        // Callbacks
        GRAPHICS_EXPORT RenderCallback& OnRender();


        GRAPHICS_EXPORT const bgfx::Stats* GetStats() const;

    private:
        Core::PIMPL<RenderEngineImpl, 1920> m_PIMPL;
    };
}