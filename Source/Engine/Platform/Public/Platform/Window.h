#pragma once

// Engine
#include <Platform/Platform.export.h>
#include <Platform/HMI/Keyboard.h>

// System
#include <cinttypes>
#include <functional>
#include <string_view>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class Keyboard;

    //////////////////////////////////////////////////////////////////////////
    class WindowArgs
    {
    public:
        PLATFORM_EXPORT WindowArgs(std::string_view title, uint32_t width, uint32_t height);

        PLATFORM_EXPORT WindowArgs();                                   // = default
        PLATFORM_EXPORT WindowArgs(const WindowArgs&);                  // = default
        PLATFORM_EXPORT WindowArgs& operator=(const WindowArgs&);       // = default
        PLATFORM_EXPORT WindowArgs(WindowArgs&&) noexcept;              // = default
        PLATFORM_EXPORT WindowArgs& operator=(WindowArgs&&) noexcept;   // = default
        PLATFORM_EXPORT ~WindowArgs();                                  // = default

    /*internal:*/
        const std::string& GetTitle() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

    private:
        std::string m_Title = std::string("AFEX");
        uint32_t m_Width = 1280;
        uint32_t m_Height = 720;
    };


    //////////////////////////////////////////////////////////////////////////
    class Window
    {
    public:
        using ResizeCallbackType = Core::Signal<void(uint32_t width, uint32_t height)>;

        // GlobalInit must be called before any window can be created.
        static PLATFORM_EXPORT bool GlobalInit();

        // When all windows are no longer needed: GlobalShutdown should be called.
        // This will close and destroy any remaining windows.
        static PLATFORM_EXPORT void GlobalShutdown();

        PLATFORM_EXPORT explicit Window(const WindowArgs& args);
        PLATFORM_EXPORT ~Window();
        PLATFORM_EXPORT bool IsValid() const { return m_PIMPL != nullptr; }

        // Window lifecycle functions:
        PLATFORM_EXPORT void RequestClose();
        PLATFORM_EXPORT bool CloseRequested() const;

        // Frame lifecycle functions.
        PLATFORM_EXPORT void PollEvents();

        // HMI:
        PLATFORM_EXPORT const Keyboard& GetKeyboard() const;
        PLATFORM_EXPORT Keyboard& GetKeyboardMutable();

        // Callbacks:
        PLATFORM_EXPORT Keyboard::KeyCallbackType& OnKeyEvent();
        PLATFORM_EXPORT Keyboard::CharCallbackType& OnCharEvent();
        PLATFORM_EXPORT ResizeCallbackType& OnResize();

        // Misc.
        PLATFORM_EXPORT void* GetNativeWindowHandle() const;

    private:
        void* m_PIMPL = nullptr;
    };
}