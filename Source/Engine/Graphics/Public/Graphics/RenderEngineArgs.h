#pragma once

// Engine
#include <Graphics/Graphics.export.h>

// System
#include <cstdint>

struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    enum class DebugMode : uint8_t;

    //////////////////////////////////////////////////////////////////////////
    class RenderEngineArgs
    {
    public:
        // nativeWindowHandle: if null the render engine will create a headless context/device
        // providing the rendering API supports it.
        GRAPHICS_EXPORT RenderEngineArgs(
            void* nativeWindowHandle,
            ImGuiContext* imguiContext,
            uint32_t width,
            uint32_t height);

        GRAPHICS_EXPORT RenderEngineArgs();                                         // = default
        GRAPHICS_EXPORT RenderEngineArgs(const RenderEngineArgs&);                  // = default
        GRAPHICS_EXPORT RenderEngineArgs& operator=(const RenderEngineArgs&);       // = default
        GRAPHICS_EXPORT RenderEngineArgs(RenderEngineArgs&&) noexcept;              // = default
        GRAPHICS_EXPORT RenderEngineArgs& operator=(RenderEngineArgs&&) noexcept;   // = default
        GRAPHICS_EXPORT ~RenderEngineArgs();                                        // = default

    /*internal:*/
        void* GetNativeWindowHandle() const;
        ImGuiContext* GetImguiContext() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
    private:
        void* m_NativeWindowHandle = nullptr;
        ImGuiContext* m_ImguiContext = nullptr;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

}