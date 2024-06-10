#pragma once

// Engine
#include <Core/PIMPL.h>
#include <Core/Signal.h>
#include <Platform/Platform.export.h>

// System
#include <cstdint>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class Keyboard;
    class Cursor;
    class WindowArgs;
    class WindowImpl;

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
        Window()                            = delete;
        Window(const Window&)               = delete;
        Window& operator=(const Window&)    = delete;
        Window(Window&&)                    = delete;
        Window& operator=(Window&&)         = delete;
        PLATFORM_EXPORT ~Window();
        PLATFORM_EXPORT bool IsValid() const;

        // Window lifecycle functions:
        PLATFORM_EXPORT void RequestClose();
        PLATFORM_EXPORT bool CloseRequested() const;

        // Frame lifecycle functions.
        PLATFORM_EXPORT void PollEvents();

        // HMI:
        PLATFORM_EXPORT const Keyboard& GetKeyboard() const;
        PLATFORM_EXPORT Keyboard& GetKeyboardMutable();
        PLATFORM_EXPORT const Cursor& GetCursor() const;
        PLATFORM_EXPORT Cursor& GetCursorMutable();

        // Size:
        PLATFORM_EXPORT ResizeCallbackType& OnResize();
        PLATFORM_EXPORT void GetSize(uint32_t& outWidth, uint32_t& outHeight) const;

        // Misc.
        PLATFORM_EXPORT void* GetNativeWindowHandle() const;

    private:
        Core::PIMPL<WindowImpl, 808> m_PIMPL;
    };
}