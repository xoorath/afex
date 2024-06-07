#include <Platform/Window.h>

// Private
#include "GLFWwindowUserData.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>

// External
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>

// System
#include <atomic>


//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////  WindowArgs public
    /*PLATFORM_EXPORT*/ WindowArgs::WindowArgs(std::string_view title, uint32_t width, uint32_t height)
        : m_Title(title)
        , m_Width(width)
        , m_Height(height)
    {
    }

    /*PLATFORM_EXPORT*/ WindowArgs::WindowArgs()                                    = default;
    /*PLATFORM_EXPORT*/ WindowArgs::WindowArgs(const WindowArgs&)                   = default;
    /*PLATFORM_EXPORT*/ WindowArgs& WindowArgs::operator=(const WindowArgs&)        = default;
    /*PLATFORM_EXPORT*/ WindowArgs::WindowArgs(WindowArgs&&) noexcept               = default;
    /*PLATFORM_EXPORT*/ WindowArgs& WindowArgs::operator=(WindowArgs&&) noexcept    = default;
    /*PLATFORM_EXPORT*/ WindowArgs::~WindowArgs()                                   = default;

    //////////////////////////////////////////////////////////////////////////  WindowArgs internal
    const std::string& WindowArgs::GetTitle() const { return m_Title; }
    uint32_t WindowArgs::GetWidth() const { return m_Width; }
    uint32_t WindowArgs::GetHeight() const { return m_Height; }

    ////////////////////////////////////////////////////////////////////////// internal
    class WindowImpl
    {

    public:
        explicit WindowImpl(GLFWwindow* window)
            : m_Window(window)
            , m_Keyboard(m_Window)
            , m_GLFWwindowUserData(m_Keyboard)
        {
            AFEX_ASSERT(m_Window != nullptr);

            glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&m_GLFWwindowUserData));

            int width, height;
            glfwGetWindowSize(m_Window, &width, &height);
            m_PreviousWidth = static_cast<uint32_t>(width);
            m_PreviousHeight = static_cast<uint32_t>(height);
        }

        ~WindowImpl()
        {
            glfwDestroyWindow(m_Window);
        }

        static void ErrorCallback(int error_code, const char* description)
        {
            AFEX_UNUSED(error_code);
            AFEX_UNUSED(description);
            AFEX_LOG_ERROR("GLFW Error ({}): {}", error_code, description);
        }

        static inline WindowImpl* Create(const WindowArgs& args);

        inline void RequestClose()
        {
            glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
        }

        inline bool CloseRequested() const
        {
            return glfwWindowShouldClose(m_Window);
        }

        inline void PollEvents()
        {
            glfwPollEvents();

            if(m_ResizeCallback)
            {
                int width, height;
                glfwGetWindowSize(m_Window, &width, &height);
                uint32_t newWidth = static_cast<uint32_t>(width);
                uint32_t newHeight = static_cast<uint32_t>(height);
                if(newWidth != m_PreviousWidth || newHeight != m_PreviousHeight)
                {
                    m_PreviousWidth = newWidth;
                    m_PreviousHeight = newHeight;
                    m_ResizeCallback.Invoke(m_PreviousWidth, m_PreviousHeight);
                }
            }
        }

        inline const Keyboard& GetKeyboard() const
        {
            return m_Keyboard;
        }

        inline Keyboard& GetKeyboardMutable()
        {
            return m_Keyboard;
        }

        inline Keyboard::KeyCallbackType& OnKeyEvent()
        {
            return GetKeyboardMutable().OnKeyEvent();
        }

        inline Keyboard::CharCallbackType& OnCharEvent()
        {
            return GetKeyboardMutable().OnCharEvent();
        }

        inline Window::ResizeCallbackType& OnResize()
        {
            return m_ResizeCallback;
        }

        void* GetNativeWindowHandle() const
        {
            return glfwGetWin32Window(m_Window);
        }
    private:
        // GLFW
        GLFWwindow* m_Window = nullptr;

        // Engine
        Keyboard m_Keyboard;
        GLFWwindowUserData m_GLFWwindowUserData;

        // Callbacks
        Window::ResizeCallbackType m_ResizeCallback;
        uint32_t m_PreviousWidth;
        uint32_t m_PreviousHeight;
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

    /*PLATFORM_EXPORT explicit*/ Window::Window(const WindowArgs& args)
    {
        m_PIMPL = reinterpret_cast<void*>(WindowImpl::Create(args));
        AFEX_LOG_TRACE("Window created {} ({}, {}, {}) "
        , (IsValid() ? "successfully" : "unsuccessfully")
        , args.GetWidth()
        , args.GetHeight()
        , args.GetTitle().c_str());
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

    /*PLATFORM_EXPORT*/ Keyboard::KeyCallbackType& Window::OnKeyEvent()
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->OnKeyEvent();
    }

    /*PLATFORM_EXPORT*/ Keyboard::CharCallbackType& Window::OnCharEvent()
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->OnCharEvent();
    }

    /*PLATFORM_EXPORT*/ Window::ResizeCallbackType& Window::OnResize()
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->OnResize();
    }

    /*PLATFORM_EXPORT*/ void* Window::GetNativeWindowHandle() const
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->GetNativeWindowHandle();
    }

    ////////////////////////////////////////////////////////////////////////// internal implementation

    /*static inline*/ WindowImpl* WindowImpl::Create(const WindowArgs& args)
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
}