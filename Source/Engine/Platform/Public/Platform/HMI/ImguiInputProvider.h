#pragma once

// Engine
#include <Core/PIMPL.h>
#include <Platform/Platform.export.h>

//////////////////////////////////////////////////////////////////////////

struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class Cursor;
    class ImGuiInputProviderImpl;
    class Keyboard;

    //////////////////////////////////////////////////////////////////////////
    class ImGuiInputProvider
    {
    public:
        PLATFORM_EXPORT ImGuiInputProvider(ImGuiContext* context, Cursor& cursor, Keyboard& keyboard);
        PLATFORM_EXPORT ImGuiInputProvider(ImGuiInputProvider&&) noexcept;              /*= default*/
        PLATFORM_EXPORT ImGuiInputProvider& operator=(ImGuiInputProvider&&) noexcept;   /*= default*/
        PLATFORM_EXPORT ~ImGuiInputProvider();                                          /*= default*/
        
        ImGuiInputProvider()                                        = delete;
        ImGuiInputProvider(const ImGuiInputProvider&)               = delete;
        ImGuiInputProvider& operator=(const ImGuiInputProvider&)    = delete;

        // Resolution scale impacts cursor position
        // Note: that the scale is inverse of the resolution.
        // ie: a 2x scale means you have 1/2 the pixels.
        PLATFORM_EXPORT void SetInputScale(float scaleX, float scaleY);

    private:
        Core::PIMPL<ImGuiInputProviderImpl, 144> m_PIMPL;
    };
}