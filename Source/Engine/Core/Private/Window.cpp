#include <Core/Window.h>

// Engine
#include <Core/Logging.h>

// External
#include <GLFW/glfw3.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// internal
    namespace
    {
        class WindowImpl
        {
        public:
            explicit WindowImpl(GLFWwindow* window)
                : m_Window(window)
            {
            }

            ~WindowImpl()
            {
                glfwDestroyWindow(m_Window);
            }

            static inline WindowImpl* Create(int32_t width, int32_t height, const char* title)
            {
                GLFWwindow* glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
                if(glfwWindow == nullptr)
                {
                    AFEX_LOG_ERROR("glfwCreateWindow failed");
                    return nullptr;
                }

                WindowImpl* impl = new WindowImpl(glfwWindow);
                
                return impl;
            }

            inline void MakeWindowContextCurrent() const
            {
                glfwMakeContextCurrent(m_Window);
            }

            inline bool CloseRequested() const
            {
                return glfwWindowShouldClose(m_Window);
            }

            inline void Clear()
            {
                glClear(GL_COLOR_BUFFER_BIT);
            }

            inline void SwapBuffers()
            {
                glfwSwapBuffers(m_Window);
            }

            inline void PollEvents()
            {
                glfwPollEvents();
            }

        private:
            GLFWwindow* m_Window = nullptr;
        };
    }

    ////////////////////////////////////////////////////////////////////////// public static

    /*CORE_EXPORT static*/ bool Window::GlobalInit()
    {
        if (!glfwInit())
        {
            AFEX_LOG_ERROR("glfwInit failed");
            return false;
        }
        AFEX_LOG_TRACE("Window::GlobalInit complete");
        return true;
    }

    /*CORE_EXPORT static*/ void Window::GlobalShutdown()
    {
        glfwTerminate();
        AFEX_LOG_TRACE("Window::GlobalShutdown complete");
    }

    ////////////////////////////////////////////////////////////////////////// public

    /*CORE_EXPORT*/ Window::Window(int32_t width, int32_t height, const char* title)
    {
        m_PIMPL = reinterpret_cast<void*>(WindowImpl::Create(width, height, title));
        AFEX_LOG_TRACE("Window created ({}, {}, {}) -> {}", width, height, title, (this->IsValid() ? "successfully" : "unsuccessfully"));
    }

    /*CORE_EXPORT*/ Window::~Window()
    {
        AFEX_LOG_TRACE("Window destroyed");
        delete reinterpret_cast<WindowImpl*>(m_PIMPL);
    }

    /*CORE_EXPORT*/ void Window::MakeWindowContextCurrent() const
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->MakeWindowContextCurrent();
    }

    /*CORE_EXPORT*/ bool Window::CloseRequested() const
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->CloseRequested();
    }

    /*CORE_EXPORT*/ void Window::Clear()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->Clear();
    }

    /*CORE_EXPORT*/ void Window::SwapBuffers()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->SwapBuffers();
    }

    /*CORE_EXPORT*/ void Window::PollEvents()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->PollEvents();
    }
}