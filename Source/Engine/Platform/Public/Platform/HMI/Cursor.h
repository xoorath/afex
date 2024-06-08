#pragma once

// Engine
#include <Core/Signal.h>
#include <Platform/Platform.export.h>

//////////////////////////////////////////////////////////////////////////

typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////
    class Cursor
    {
    public:
        using CursorPositionCallbackType = Core::Signal<void(float xPos, float yPos)>;
        using CursorEnterCallbackType = Core::Signal<void(int entered)>;
        using CursorButtonCallbackType = Core::Signal<void(int button, int action, int mods)>;
        using CursorScrollCallbackType = Core::Signal<void(float xScroll, float yScroll)>;

        PLATFORM_EXPORT CursorPositionCallbackType& OnCursorPosition();
        PLATFORM_EXPORT CursorEnterCallbackType& OnCursorEnter();
        PLATFORM_EXPORT CursorButtonCallbackType& OnCursorButton();
        PLATFORM_EXPORT CursorScrollCallbackType& OnCursorScroll();

    /*Internal:*/
        void GLFWcursorposfun(double xpos, double ypos);
        void GLFWcursorenterfun(int entered);
        void GLFWmousebuttonfun(int button, int action, int mods);
        void GLFWscrollfun(double xoffset, double yoffset);
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

        void* m_PIMPL;
    };
}