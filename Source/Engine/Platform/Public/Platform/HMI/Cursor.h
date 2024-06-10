#pragma once

// Engine
#include <Core/PIMPL.h>
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
        // Only the WindowImpl is allowed to create & initialize a Cursor
        friend class WindowImpl;
        Cursor();                               /*=default*/
        Cursor(const Cursor&)                   = delete;
        Cursor& operator=(const Cursor&)        = delete;
        Cursor(Cursor&&) noexcept;              /*=default*/
        Cursor& operator=(Cursor&&) noexcept;   /*=default*/
        ~Cursor();                              /*=default*/

        void Init(GLFWwindow* window);

        Core::PIMPL<CursorImpl, 424> m_PIMPL;
    };
}