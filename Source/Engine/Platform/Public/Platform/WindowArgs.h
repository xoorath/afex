#pragma once

// Engine
#include <Platform/Platform.export.h>

// System
#include <cstdint>
#include <string_view>

//////////////////////////////////////////////////////////////////////////
struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////
    // These arguments are provided to the Window constructor and its values
    // are used for initialization. To change aspects such as window size
    // or title at runtime: that should be done through a reference to the
    // Window itself.
    class WindowArgs
    {
    public:
        PLATFORM_EXPORT WindowArgs(
            std::string_view title,
            ImGuiContext* context,
            uint32_t width,
            uint32_t height);

        PLATFORM_EXPORT WindowArgs();                                   // = default
        PLATFORM_EXPORT WindowArgs(const WindowArgs&);                  // = default
        PLATFORM_EXPORT WindowArgs& operator=(const WindowArgs&);       // = default
        PLATFORM_EXPORT WindowArgs(WindowArgs&&) noexcept;              // = default
        PLATFORM_EXPORT WindowArgs& operator=(WindowArgs&&) noexcept;   // = default
        PLATFORM_EXPORT ~WindowArgs();                                  // = default

    /*internal:*/
        const std::string& GetTitle() const;
        ImGuiContext* GetImguiContextMutable() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

    private:
        std::string m_Title = std::string("AFEX");
        ImGuiContext* m_ImguiContext = nullptr;
        uint32_t m_Width = 1280;
        uint32_t m_Height = 720;
    };
}