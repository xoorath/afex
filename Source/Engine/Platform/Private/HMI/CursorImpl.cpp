#include "CursorImpl.h"

// Platform
#include "../GLFWwindowUserData.h"

// External
#include <GLFW/glfw3.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*explicit*/ CursorImpl::CursorImpl(GLFWwindow* window)
        : m_Window(window)
        , m_OnCursorPosition()
        , m_OnCursorEnter()
        , m_OnMouseButton()
        , m_OnScroll()
    {
        if(m_Window != nullptr)
        {
            glfwSetCursorPosCallback(window, 
                [](GLFWwindow* window, double xpos, double ypos)
                {
                    auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
                    auto self = reinterpret_cast<CursorImpl*>(userData->GetCursorPIMPL());
                    self->GLFWcursorposfun(xpos, ypos);
                });

            glfwSetCursorEnterCallback(window, 
                [](GLFWwindow* window, int entered)
                {
                    auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
                    auto self = reinterpret_cast<CursorImpl*>(userData->GetCursorPIMPL());
                    self->GLFWcursorenterfun(entered);
                });

            glfwSetMouseButtonCallback(window, 
                [](GLFWwindow* window, int button, int action, int mods)
                {
                    auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
                    auto self = reinterpret_cast<CursorImpl*>(userData->GetCursorPIMPL());
                    self->GLFWmousebuttonfun(button, action, mods);
                });
            glfwSetScrollCallback(window, 
                [](GLFWwindow* window, double xoffset, double yoffset)
                {
                    auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
                    auto self = reinterpret_cast<CursorImpl*>(userData->GetCursorPIMPL());
                    self->GLFWscrollfun(xoffset, yoffset);
                });
        }
    }

    CursorImpl::~CursorImpl()
    {
        if(m_Window != nullptr)
        {
            glfwSetCursorPosCallback(m_Window, nullptr);
            glfwSetCursorEnterCallback(m_Window, nullptr);
            glfwSetMouseButtonCallback(m_Window, nullptr);
            glfwSetScrollCallback(m_Window, nullptr);
        }
    }

    CursorImpl::CursorImpl(CursorImpl&& other) noexcept
        : m_Window(other.m_Window)
        , m_OnCursorPosition(std::move(other.m_OnCursorPosition))
        , m_OnCursorEnter(std::move(other.m_OnCursorEnter))
        , m_OnMouseButton(std::move(other.m_OnMouseButton))
        , m_OnScroll(std::move(other.m_OnScroll))
    {
        other.m_Window = nullptr;
    }

    CursorImpl& CursorImpl::operator=(CursorImpl&& other) noexcept
    {
        m_Window = other.m_Window;
        m_OnCursorPosition = std::move(other.m_OnCursorPosition);
        m_OnCursorEnter = std::move(other.m_OnCursorEnter);
        m_OnMouseButton = std::move(other.m_OnMouseButton);
        m_OnScroll = std::move(other.m_OnScroll);
        other.m_Window = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////// Private
    void CursorImpl::GLFWcursorposfun(double xpos, double ypos)
    {
        m_OnCursorPosition.Invoke(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    void CursorImpl::GLFWcursorenterfun(int entered)
    {
        m_OnCursorEnter.Invoke(entered);
    }

    void CursorImpl::GLFWmousebuttonfun(int button, int action, int mods)
    {
        m_OnMouseButton(button, action, mods);
    }

    void CursorImpl::GLFWscrollfun(double xoffset, double yoffset)
    {
        m_OnScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
    }
}