#include <Platform/Window.h>

// Private
#include "GLFWwindowUserData.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>
#include <Graphics/RenderEngine.h>
#include <Graphics/DebugMode.h>

// External
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>

// System
#include <atomic>


//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// internal
    class WindowImpl
    {

    public:
        explicit WindowImpl(GLFWwindow* window, Graphics::RenderEngine&& engine)
            : m_Window(window)
            , m_Keyboard(m_Window)
            , m_GLFWwindowUserData(m_Keyboard)
            , m_RenderEngine(std::move(engine))
        {
            AFEX_ASSERT(m_Window != nullptr);

            glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&m_GLFWwindowUserData));

            glfwGetWindowSize(window, &m_PreviousWidth, &m_PreviousHeight);

            m_RenderEngine.SetDebugMode(Graphics::DebugMode::Text);
            m_Keyboard.OnKeyEvent().Add([this](KeyCode key, int32_t scanCode, KeyboardAction action, int32_t modifiers)
            {
                AFEX_UNUSED(scanCode);
                AFEX_UNUSED(modifiers);
                if(action == KeyboardAction::Press)
                {
                    switch(key)
                    {
                        case KeyCode::F1:
                            m_RenderEngine.SetDebugMode(Graphics::DebugMode::None);
                            break;
                        case KeyCode::F2:
                            m_RenderEngine.SetDebugMode(Graphics::DebugMode::Text);
                            break;
                        case KeyCode::F3:
                            m_RenderEngine.SetDebugMode(Graphics::DebugMode::Stats);
                            break;
                        case KeyCode::F4:
                            m_RenderEngine.SetDebugMode(Graphics::DebugMode::Wireframe);
                            break;
                        case KeyCode::F5:
                            m_RenderEngine.SetDebugMode(Graphics::DebugMode::Profiler);
                            break;
                    }
                }
            });
        }

        ~WindowImpl()
        {
            // Manually shutdown the render engine to ensure it happens before the window is destroyed.
            m_RenderEngine.Shutdown();
            glfwDestroyWindow(m_Window);
        }

        static void ErrorCallback(int error_code, const char* description)
        {
            AFEX_UNUSED(error_code);
            AFEX_UNUSED(description);
            AFEX_LOG_ERROR("GLFW Error ({}): {}", error_code, description);
        }

        static inline WindowImpl* Create(uint32_t width, uint32_t height, const char* title);

        inline void RequestClose()
        {
            glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
        }

        inline bool CloseRequested() const
        {
            return glfwWindowShouldClose(m_Window);
        }

        inline void RenderFrame()
        {
            int width, height;
            glfwGetWindowSize(m_Window, &width, &height);
            if(m_PreviousWidth != width or m_PreviousHeight != height)
            {
                m_PreviousWidth = width;
                m_PreviousHeight = height;
                m_RenderEngine.Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            }

            m_RenderEngine.RenderFrame();
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
        // GLFW
        GLFWwindow* m_Window = nullptr;

        // Graphics
        Graphics::RenderEngine m_RenderEngine;
        int m_PreviousWidth = 0;
        int m_PreviousHeight = 0;

        // Engine
        Keyboard m_Keyboard;
        GLFWwindowUserData m_GLFWwindowUserData;
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

        const GLFWerrorfun existingCallback = glfwSetErrorCallback(&WindowImpl::ErrorCallback);
        AFEX_ASSERT_MSG(existingCallback == nullptr, "GLFW already has an error callback set. This is unexpected.");

        AFEX_LOG_TRACE("Window::GlobalInit complete");
        return true;
    }

    /*PLATFORM_EXPORT static*/ void Window::GlobalShutdown()
    {
        glfwTerminate();
        AFEX_LOG_TRACE("Window::GlobalShutdown complete");
    }

    ////////////////////////////////////////////////////////////////////////// public

    /*PLATFORM_EXPORT*/ Window::Window(uint32_t width, uint32_t height, const char* title)
    {
        m_PIMPL = reinterpret_cast<void*>(WindowImpl::Create(width, height, title));
        AFEX_LOG_TRACE("Window created {} ({}, {}, {}) ", (IsValid() ? "successfully" : "unsuccessfully"), width, height, title);
    }

    /*PLATFORM_EXPORT*/ Window::~Window()
    {
        AFEX_LOG_TRACE("Window destroyed");
        delete reinterpret_cast<WindowImpl*>(m_PIMPL);
    }

    /*PLATFORM_EXPORT*/ void Window::RequestClose()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->RequestClose();
    }

    /*PLATFORM_EXPORT*/ bool Window::CloseRequested() const
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->CloseRequested();
    }

    /*PLATFORM_EXPORT*/ void Window::RenderFrame()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->RenderFrame();
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

    ////////////////////////////////////////////////////////////////////////// internal implementation

    /*static inline*/ WindowImpl* WindowImpl::Create(uint32_t width, uint32_t height, const char* title)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        GLFWwindow* glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
        AFEX_ASSERT_MSG(glfwWindow != nullptr, "glfwCreateWindow failed");
        if (glfwWindow == nullptr)
        {
            return nullptr;
        }

        Graphics::RenderEngineArgs renderEngineArgs(
            glfwGetWin32Window(glfwWindow),
            width,
            height
        );

        Graphics::RenderEngine renderEngine(renderEngineArgs);

        if(renderEngine.IsValid() == false)
        {
            glfwDestroyWindow(glfwWindow);
            return nullptr;
        }
        
        return new WindowImpl(glfwWindow, std::move(renderEngine));
    }
}