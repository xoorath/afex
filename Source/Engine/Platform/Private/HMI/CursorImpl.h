#pragma once

#include <Platform/HMI/Cursor.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////
    class CursorImpl
    {
    public:
        explicit CursorImpl(GLFWwindow* window);
        ~CursorImpl();
        
        CursorImpl(CursorImpl&&) noexcept               = delete;
        CursorImpl& operator=(CursorImpl&&) noexcept    = delete;
        CursorImpl()                                    = delete;
        CursorImpl(const CursorImpl&)                   = delete;
        CursorImpl& operator=(const CursorImpl&)        = delete;

        Cursor::CursorPositionCallbackType& OnCursorPosition()  { return m_OnCursorPosition; }
        Cursor::CursorEnterCallbackType&    OnCursorEnter()     { return m_OnCursorEnter; }
        Cursor::CursorButtonCallbackType&   OnCursorButton()    { return m_OnCursorButton; }
        Cursor::CursorScrollCallbackType&   OnCursorScroll()    { return m_OnCursorScroll; }

        // Callbacks that come from Cursor
        void GLFWcursorposfun(double xpos, double ypos);
        void GLFWcursorenterfun(int entered);
        void GLFWmousebuttonfun(int button, int action, int mods);
        void GLFWscrollfun(double xoffset, double yoffset);

    private:
        static void GLFWcursorposfunStatic(GLFWwindow* window, double xpos, double ypos);
        static void GLFWcursorenterfunStatic(GLFWwindow* window, int entered);
        static void GLFWmousebuttonfunStatic(GLFWwindow* window, int button, int action, int mods);
        static void GLFWscrollfunStatic(GLFWwindow* window, double xoffset, double yoffset);

        GLFWwindow* m_Window;

        Cursor::CursorPositionCallbackType m_OnCursorPosition;
        Cursor::CursorEnterCallbackType m_OnCursorEnter;
        Cursor::CursorButtonCallbackType m_OnCursorButton;
        Cursor::CursorScrollCallbackType m_OnCursorScroll;

    };
}