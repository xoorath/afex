#pragma once

// Engine
#include <Platform/Platform.export.h>
#include <Platform/HMI/Keyboard.h>

// System
#include <inttypes.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class Keyboard;

    //////////////////////////////////////////////////////////////////////////
    class Window
    {
    public:
        // GlobalInit must be called before any window can be created.
        static PLATFORM_EXPORT bool GlobalInit();

        // When all windows are no longer needed: GlobalShutdown should be called.
        // This will close and destroy any remaining windows.
        static PLATFORM_EXPORT void GlobalShutdown();

        PLATFORM_EXPORT Window(int32_t width, int32_t height, const char* title);
        PLATFORM_EXPORT ~Window();
        PLATFORM_EXPORT bool IsValid() const { return m_PIMPL != nullptr; }

        // The following window functions are only safe to call if IsValid returns true.

        PLATFORM_EXPORT void MakeWindowContextCurrent() const;
        PLATFORM_EXPORT void RequestClose();
        PLATFORM_EXPORT bool CloseRequested() const;
        PLATFORM_EXPORT void Clear();
        PLATFORM_EXPORT void SwapBuffers();
        PLATFORM_EXPORT void PollEvents();

        PLATFORM_EXPORT const Keyboard& GetKeyboard() const;
        PLATFORM_EXPORT Keyboard& GetKeyboardMutable();

    private:
        void* m_PIMPL = nullptr;
    };
}