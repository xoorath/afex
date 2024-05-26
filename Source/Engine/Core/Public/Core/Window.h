#pragma once

// Engine
#include <Core/Core.export.h>

// System
#include <inttypes.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class Window
    {
    public:
        // GlobalInit must be called before any window can be created.
        static CORE_EXPORT bool GlobalInit();

        // When all windows are no longer needed: GlobalShutdown should be called.
        // This will close and destroy any remaining windows.
        static CORE_EXPORT void GlobalShutdown();

        CORE_EXPORT Window(int32_t width, int32_t height, const char* title);
        CORE_EXPORT ~Window();
        CORE_EXPORT bool IsValid() const { return m_PIMPL != nullptr; }

        // The following window functions are only safe to call if IsValid returns true.

        CORE_EXPORT void MakeWindowContextCurrent() const;
        CORE_EXPORT bool CloseRequested() const;
        CORE_EXPORT void Clear();
        CORE_EXPORT void SwapBuffers();
        CORE_EXPORT void PollEvents();
    private:
        void* m_PIMPL = nullptr;
    };
}