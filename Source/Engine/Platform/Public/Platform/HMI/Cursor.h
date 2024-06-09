#pragma once

// Engine
#include <Core/PIMPL.h>
#include <Core/Signal.h>
#include <Platform/HMI/CursorCallbackTypes.h>
#include <Platform/Platform.export.h>

//////////////////////////////////////////////////////////////////////////

typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class CursorImpl;

    //////////////////////////////////////////////////////////////////////////
    // The cursor HMI bundles together cursors, mouse buttons and scroll
    // - the functionality we associate with computer mice.
    class Cursor
    {
    public:
        PLATFORM_EXPORT CursorPositionCallbackType& OnCursorPosition();
        PLATFORM_EXPORT CursorEnterCallbackType& OnCursorEnter();
        PLATFORM_EXPORT MouseButtonCallbackType& OnMouseButton();
        PLATFORM_EXPORT ScrollCallbackType& OnScroll();

    /*internal:*/
        CursorImpl* GetPIMPL(); // Exposed to be provided via glfw user data
    private:
        // Only the WindowImpl is allowed to create a Keyboard
        friend class WindowImpl;

        // Sets up cursor bindings for the window.
        // A null-pattern implementation can be created by passing a null window
        explicit Cursor(GLFWwindow* window);

        Cursor(Cursor&&) noexcept;
        Cursor& operator=(Cursor&&) noexcept;
        ~Cursor();
        
        Cursor() = delete;
        Cursor(const Cursor&) = delete;
        Cursor& operator=(const Cursor&) = delete;

        Core::PIMPL<CursorImpl, 424> m_PIMPL;
    };
}