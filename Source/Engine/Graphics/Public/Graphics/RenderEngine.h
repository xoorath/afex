#pragma once

// Engine
#include <Graphics/Graphics.export.h>

// System
#include <cstdint>

namespace Graphics
{
    enum class DebugMode : uint8_t;

    // RenderEngineArgs should be provided during render engine startup
    class RenderEngineArgs
    {
    public:
        // nativeWindowHandle: if null the render engine will create a headless context/device
        // providing the rendering API supports it.
        GRAPHICS_EXPORT RenderEngineArgs(void* nativeWindowHandle, uint32_t width, uint32_t height);

        GRAPHICS_EXPORT RenderEngineArgs();                                         // = default
        GRAPHICS_EXPORT RenderEngineArgs(const RenderEngineArgs&);                  // = default
        GRAPHICS_EXPORT RenderEngineArgs& operator=(const RenderEngineArgs&);       // = default
        GRAPHICS_EXPORT RenderEngineArgs(RenderEngineArgs&&) noexcept;              // = default
        GRAPHICS_EXPORT RenderEngineArgs& operator=(RenderEngineArgs&&) noexcept;   // = default
        GRAPHICS_EXPORT ~RenderEngineArgs();                                        // = default

    /*internal:*/ 
        void* GetNativeWindowHandle() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
    private:
        void* m_NativeWindowHandle = nullptr;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

    // The RenderEngine manages our interactions with the rendering backend (bgfx)
    // and sets up the render thread.
    // Any *one* thread may submit draw calls. Whichever thread is drawing may then
    // call RenderFrame to conclude 
    class RenderEngine
    {
    public:
        GRAPHICS_EXPORT explicit RenderEngine(const RenderEngineArgs& args);
        GRAPHICS_EXPORT ~RenderEngine();
        GRAPHICS_EXPORT RenderEngine(RenderEngine&& other) noexcept;
        GRAPHICS_EXPORT RenderEngine& operator=(RenderEngine&& other) noexcept;
        GRAPHICS_EXPORT bool IsValid() const;
        GRAPHICS_EXPORT void Shutdown();

        // Only one thread is permitted to submit render calls.
        // Whichever thread that is should call RenderFrame when rendering is complete for the frame.
        GRAPHICS_EXPORT void RenderFrame() const;

        // (thread safe) signals the render engine to resize the render target
        GRAPHICS_EXPORT void Resize(uint32_t width, uint32_t height);
        // (thread safe) sets the bgfx debug mode
        GRAPHICS_EXPORT void SetDebugMode(DebugMode mode);

        RenderEngine(const RenderEngine&) = delete;
        RenderEngine& operator=(const RenderEngine&) = delete;

    /*internal:*/
        static constexpr uint32_t k_ShutdownTimeoutMilliseconds = 1000;

        // Stack size, if zero is passed it will use OS default thread stack
        static constexpr uint32_t k_RenderThreadStackSize = 0;

        // After this time the frame dropped
        // A value of -1 will use the bgfx default value of 5000 ms
        static constexpr uint32_t k_RenderTimeoutMilliseconds = 33;

    private:
        void* m_PIMPL = nullptr;
    };
}