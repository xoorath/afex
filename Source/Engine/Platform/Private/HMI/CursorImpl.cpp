#include "CursorImpl.h"

// Platform
#include "../GLFWwindowUserData.h"

// Engine
#include <Core/Assert.h>

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
        , m_OnCursorButton()
        , m_OnCursorScroll()
    {
        if(m_Window != nullptr)
        {
            AFEX_ASSERT_MSG(glfwSetCursorPosCallback(window, &CursorImpl::GLFWcursorposfunStatic) == nullptr,
                "Chaining existing callbacks is not implemented");
            AFEX_ASSERT_MSG(glfwSetCursorEnterCallback(window, &CursorImpl::GLFWcursorenterfunStatic) == nullptr,
                "Chaining existing callbacks is not implemented");
            AFEX_ASSERT_MSG(glfwSetMouseButtonCallback(window, &CursorImpl::GLFWmousebuttonfunStatic) == nullptr,
                "Chaining existing callbacks is not implemented");
            AFEX_ASSERT_MSG(glfwSetScrollCallback(window, &CursorImpl::GLFWscrollfunStatic) == nullptr,
                "Chaining existing callbacks is not implemented");
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

    ////////////////////////////////////////////////////////////////////////// Private static
    /*static*/ void CursorImpl::GLFWcursorposfunStatic(GLFWwindow* window, double xpos, double ypos)
    {
        auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
        userData->GetCursorMutable().GLFWcursorposfun(xpos, ypos);
    }

    /*static*/ void CursorImpl::GLFWcursorenterfunStatic(GLFWwindow* window, int entered)
    {
        auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
        userData->GetCursorMutable().GLFWcursorenterfun(entered);
    }

    /*static*/ void CursorImpl::GLFWmousebuttonfunStatic(GLFWwindow* window, int button, int action, int mods)
    {
        auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
        userData->GetCursorMutable().GLFWmousebuttonfun(button, action, mods);
    }

    /*static*/ void CursorImpl::GLFWscrollfunStatic(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
        userData->GetCursorMutable().GLFWscrollfun(xoffset, yoffset);
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
        m_OnCursorButton(button, action, mods);
    }

    void CursorImpl::GLFWscrollfun(double xoffset, double yoffset)
    {
        m_OnCursorScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
    }
}