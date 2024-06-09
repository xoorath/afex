#pragma once

#include <Platform/HMI/CursorCallbackTypes.h>

//////////////////////////////////////////////////////////////////////////

typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////
    class CursorImpl
    {
    public:
        explicit CursorImpl(GLFWwindow* window);
        CursorImpl(CursorImpl&&) noexcept;
        CursorImpl& operator=(CursorImpl&&) noexcept;
        CursorImpl()                                    = default;
        CursorImpl(const CursorImpl&)                   = delete;
        CursorImpl& operator=(const CursorImpl&)        = delete;
        ~CursorImpl();

        CursorPositionCallbackType& OnCursorPosition()  { return m_OnCursorPosition; }
        CursorEnterCallbackType&    OnCursorEnter()     { return m_OnCursorEnter; }
        MouseButtonCallbackType&    OnMouseButton()     { return m_OnMouseButton; }
        ScrollCallbackType&         OnScroll()          { return m_OnScroll; }

    private:
        void GLFWcursorposfun(double xpos, double ypos);
        void GLFWcursorenterfun(int entered);
        void GLFWmousebuttonfun(int button, int action, int mods);
        void GLFWscrollfun(double xoffset, double yoffset);

        GLFWwindow* m_Window = nullptr;

        CursorPositionCallbackType m_OnCursorPosition;
        CursorEnterCallbackType m_OnCursorEnter;
        MouseButtonCallbackType m_OnMouseButton;
        ScrollCallbackType m_OnScroll;
    };
}