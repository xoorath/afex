#pragma once

// Engine
#include <Platform/Platform.export.h>

//////////////////////////////////////////////////////////////////////////

struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class Cursor;
    class Keyboard;

    //////////////////////////////////////////////////////////////////////////
    class ImGuiInputProvider
    {
    public:
        PLATFORM_EXPORT ImGuiInputProvider(ImGuiContext* context, Cursor& cursor, Keyboard& keyboard);
        PLATFORM_EXPORT ImGuiInputProvider(ImGuiInputProvider&&) noexcept;
        PLATFORM_EXPORT ImGuiInputProvider& operator=(ImGuiInputProvider&&) noexcept;
        PLATFORM_EXPORT ~ImGuiInputProvider(); /*= default*/
        
        ImGuiInputProvider()                                        = delete;
        ImGuiInputProvider(const ImGuiInputProvider&)               = delete;
        ImGuiInputProvider& operator=(const ImGuiInputProvider&)    = delete;
        
    private:
        void* m_PIMPL;

    };
}