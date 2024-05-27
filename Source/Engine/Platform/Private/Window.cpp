#include <Platform/Window.h>

// Private
#include "GLFWwindowUserData.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>

// External
#include <GLFW/glfw3.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// internal
    class WindowImpl
    {
    public:
        explicit WindowImpl(GLFWwindow* window)
            : m_Window(window)
            , m_Keyboard(m_Window)
            , m_UserData(m_Keyboard)
        {
            glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&m_UserData));
        }

        ~WindowImpl()
        {
            glfwDestroyWindow(m_Window);
        }

        static inline WindowImpl* Create(int32_t width, int32_t height, const char* title)
        {
            GLFWwindow* glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
            AFEX_ASSERT_MSG(glfwWindow != nullptr, "glfwCreateWindow failed");
            if(glfwWindow == nullptr)
            {
                return nullptr;
            }

            WindowImpl* impl = new WindowImpl(glfwWindow);
                
            return impl;
        }

        inline void MakeWindowContextCurrent() const
        {
            glfwMakeContextCurrent(m_Window);
        }

        inline void RequestClose()
        {
            glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
        }

        inline bool CloseRequested() const
        {
            return glfwWindowShouldClose(m_Window);
        }

        inline void Clear()
        {
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.0f);
        }

        inline void SwapBuffers()
        {
            glfwSwapBuffers(m_Window);
        }

        inline void PollEvents()
        {
            glfwPollEvents();
        }

        inline const Keyboard& GetKeyboard() const
        {
            return m_Keyboard;
        }

        inline Keyboard& GetKeyboardMutable()
        {
            return m_Keyboard;
        }

    private:
        GLFWwindow* m_Window = nullptr;

        Keyboard m_Keyboard;
        GLFWwindowUserData m_UserData;
    };

    ////////////////////////////////////////////////////////////////////////// public static

    /*PLATFORM_EXPORT static*/ bool Window::GlobalInit()
    {
        const int glfwInitResult = glfwInit();
        AFEX_ASSERT_MSG(glfwInitResult == GLFW_TRUE, "glfwCreateWindow failed");
        if (glfwInitResult != GLFW_TRUE)
        {
            return false;
        }

        AFEX_LOG_TRACE("Window::GlobalInit complete");
        return true;
    }

    /*PLATFORM_EXPORT static*/ void Window::GlobalShutdown()
    {
        glfwTerminate();
        AFEX_LOG_TRACE("Window::GlobalShutdown complete");
    }

    ////////////////////////////////////////////////////////////////////////// public

    /*PLATFORM_EXPORT*/ Window::Window(int32_t width, int32_t height, const char* title)
    {
        m_PIMPL = reinterpret_cast<void*>(WindowImpl::Create(width, height, title));
        AFEX_LOG_TRACE("Window created ({}, {}, {}) -> {}", width, height, title, (this->IsValid() ? "successfully" : "unsuccessfully"));
    }

    /*PLATFORM_EXPORT*/ Window::~Window()
    {
        AFEX_LOG_TRACE("Window destroyed");
        delete reinterpret_cast<WindowImpl*>(m_PIMPL);
    }

    /*PLATFORM_EXPORT*/ void Window::MakeWindowContextCurrent() const
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->MakeWindowContextCurrent();
    }

    /*PLATFORM_EXPORT*/ void Window::RequestClose()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->RequestClose();
    }

    /*PLATFORM_EXPORT*/ bool Window::CloseRequested() const
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->CloseRequested();
    }

    /*PLATFORM_EXPORT*/ void Window::Clear()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->Clear();
    }

    /*PLATFORM_EXPORT*/ void Window::SwapBuffers()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->SwapBuffers();
    }

    /*PLATFORM_EXPORT*/ void Window::PollEvents()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->PollEvents();
    }

    /*PLATFORM_EXPORT*/ const Keyboard& Window::GetKeyboard() const
    {
        return reinterpret_cast<const WindowImpl*>(m_PIMPL)->GetKeyboard();
    }

    /*PLATFORM_EXPORT*/ Keyboard& Window::GetKeyboardMutable()
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->GetKeyboardMutable();
    }
}