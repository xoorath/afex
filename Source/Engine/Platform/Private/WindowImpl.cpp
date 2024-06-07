#include "WindowImpl.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>
#include <Platform/WindowArgs.h>

// External
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>

// System
#include <atomic>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{

    ////////////////////////////////////////////////////////////////////////// public static
    /*static*/ WindowImpl* WindowImpl::Create(const WindowArgs& args)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        GLFWwindow* const glfwWindow = glfwCreateWindow(
            /*width=*/      args.GetWidth(),
            /*height=*/     args.GetHeight(),
            /*title=*/      args.GetTitle().c_str(),
            /*monitor=*/    nullptr,
            /*share=*/      nullptr);

        AFEX_ASSERT_MSG(glfwWindow != nullptr, "glfwCreateWindow failed");
        if (glfwWindow == nullptr)
        {
            return nullptr;
        }

        return new WindowImpl(glfwWindow);
    }

    /*static*/ void WindowImpl::ErrorCallback(int error_code, const char* description)
    {
        AFEX_UNUSED(error_code);
        AFEX_UNUSED(description);
        AFEX_LOG_ERROR("GLFW Error ({}): {}", error_code, description);
    }

    /*static*/ bool WindowImpl::GlobalInit()
    {
        const int glfwInitResult = glfwInit();
        AFEX_ASSERT_MSG(glfwInitResult == GLFW_TRUE, "glfwCreateWindow failed");
        if (glfwInitResult != GLFW_TRUE)
        {
            return false;
        }

        const GLFWerrorfun existingCallback = glfwSetErrorCallback(&WindowImpl::ErrorCallback);
        AFEX_ASSERT_MSG(existingCallback == nullptr, "GLFW already has an error callback set. This is unexpected.");

        AFEX_LOG_TRACE("Window::GlobalInit complete");
        return true;
    }

    /*static*/ void WindowImpl::GlobalShutdown()
    {
        glfwTerminate();
        AFEX_LOG_TRACE("Window::GlobalShutdown complete");
    }

    ////////////////////////////////////////////////////////////////////////// public

    /*explicit*/ WindowImpl::WindowImpl(GLFWwindow* window)
        : m_Window(window)
        , m_Keyboard(m_Window)
        , m_GLFWwindowUserData(m_Keyboard)
        , m_ResizeCallback()
        , m_PreviousWidth(0)
        , m_PreviousHeight(0)
    {
        AFEX_ASSERT(m_Window != nullptr);

        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&m_GLFWwindowUserData));

        int width, height;
        glfwGetWindowSize(m_Window, &width, &height);
        m_PreviousWidth = static_cast<uint32_t>(width);
        m_PreviousHeight = static_cast<uint32_t>(height);
    }

    WindowImpl::~WindowImpl()
    {
        glfwDestroyWindow(m_Window);
    }


    void WindowImpl::RequestClose()
    {
        glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
    }

    bool WindowImpl::CloseRequested() const
    {
        return glfwWindowShouldClose(m_Window);
    }

    void WindowImpl::PollEvents()
    {
        glfwPollEvents();

        if (m_ResizeCallback)
        {
            int width, height;
            glfwGetWindowSize(m_Window, &width, &height);
            auto newWidth  = static_cast<uint32_t>(width);
            auto newHeight = static_cast<uint32_t>(height);
            if (newWidth != m_PreviousWidth || newHeight != m_PreviousHeight)
            {
                m_PreviousWidth = newWidth;
                m_PreviousHeight = newHeight;
                m_ResizeCallback.Invoke(m_PreviousWidth, m_PreviousHeight);
            }
        }
    }

    const Keyboard& WindowImpl::GetKeyboard() const
    {
        return m_Keyboard;
    }

    Keyboard& WindowImpl::GetKeyboardMutable()
    {
        return m_Keyboard;
    }

    Keyboard::KeyCallbackType& WindowImpl::OnKeyEvent()
    {
        return GetKeyboardMutable().OnKeyEvent();
    }

    Keyboard::CharCallbackType& WindowImpl::OnCharEvent()
    {
        return GetKeyboardMutable().OnCharEvent();
    }

    Window::ResizeCallbackType& WindowImpl::OnResize()
    {
        return m_ResizeCallback;
    }

    void* WindowImpl::GetNativeWindowHandle() const
    {
        return glfwGetWin32Window(m_Window);
    }
}