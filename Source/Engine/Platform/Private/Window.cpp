#include <Platform/Window.h>

// Private
#include "GLFWwindowUserData.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>

// External
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/handlealloc.h>
#include <bx/thread.h>
#include <bx/mutex.h>
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
        explicit WindowImpl(GLFWwindow* window)
            : m_Window(window)
            , m_Keyboard(m_Window)
            , m_GLFWwindowUserData(m_Keyboard)
        {
            AFEX_ASSERT(m_Window != nullptr);

            glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&m_GLFWwindowUserData));
        }

        ~WindowImpl()
        {
            m_QuitRenderThread.store(true);
            if(m_RenderThread.isRunning())
            {
                while (bgfx::RenderFrame::NoContext != bgfx::renderFrame(/*timeoutMs=*/1000)) {}
                m_RenderThread.shutdown();
                AFEX_ASSERT_MSG(m_RenderThread.getExitCode() == 0, "Render thread shutdown was not successful");
            }
            glfwDestroyWindow(m_Window);
        }

        static void ErrorCallback(int error_code, const char* description)
        {
            AFEX_LOG_ERROR("GLFW Error ({}): {}", error_code, description);
        }

        static inline WindowImpl* Create(int32_t width, int32_t height, const char* title);
        inline void MakeWindowContextCurrent() const
        {
            //glfwMakeContextCurrent(m_Window);
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

        }

        inline void SwapBuffers()
        {
            bgfx::renderFrame();
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
        static int32_t StaticRenderThread(bx::Thread* thread, void* userData);
        int32_t RenderThread(bx::Thread* thread);

        // 0 instructs bgfx to use the OS default stack size
        static constexpr uint32_t k_RenderThreadStackSize = 0;

        // GLFW
        GLFWwindow* m_Window = nullptr;

        // bgfx
        bx::Thread m_RenderThread;
        std::atomic_bool m_QuitRenderThread = false;
        bgfx::PlatformData m_PlatformData;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        static const uint8_t s_Logo[4000];

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

    ////////////////////////////////////////////////////////////////////////// internal implementation

    /*static inline*/ WindowImpl* WindowImpl::Create(int32_t width, int32_t height, const char* title)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        GLFWwindow* glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
        AFEX_ASSERT_MSG(glfwWindow != nullptr, "glfwCreateWindow failed");
        if (glfwWindow == nullptr)
        {
            return nullptr;
        }

        // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
        // Most graphics APIs must be used on the same thread that created the window.
        bgfx::renderFrame();
        
        // After this point we setup the render thread. If this step fails we will free the WindowImpl.
        // We do this because the WindowImpl is the user data for the render thread.
        WindowImpl* impl = new WindowImpl(glfwWindow);

        impl->m_PlatformData.nwh = reinterpret_cast<void*>(glfwGetWin32Window(glfwWindow));
        int actualWindowWidth, actualWindowHeight;
        glfwGetWindowSize(glfwWindow, &actualWindowWidth, &actualWindowHeight);
        impl->m_Width = static_cast<uint32_t>(actualWindowWidth);
        impl->m_Height = static_cast<uint32_t>(actualWindowHeight);

        if(!impl->m_RenderThread.init(
            /*fn=*/        &WindowImpl::StaticRenderThread,
            /*userData=*/  reinterpret_cast<void*>(impl),
            /*stackSize=*/ k_RenderThreadStackSize,
            /*name=*/      "Render Thread"))
        {
            AFEX_LOG_ERROR("Failed to create the render thread");
            delete impl; // note: ~impl will destroy the GLFWwindow
            return nullptr;
        }
        return impl;
    }


    int32_t WindowImpl::RenderThread(bx::Thread* thread)
    {
        AFEX_UNUSED(thread);
        bgfx::Init init;
        init.platformData = m_PlatformData;
        init.resolution.width = m_Width;
        init.resolution.height = m_Height;
        init.resolution.reset = BGFX_RESET_VSYNC;
        if(!bgfx::init(init))
        {
            AFEX_LOG_ERROR("(Render thread) Failed to init bgfx");
            return -1;
        }

        const bgfx::ViewId kClearView = 0;
        bgfx::setViewClear(kClearView, 0, BGFX_CLEAR_COLOR);
        bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);
        uint32_t width = m_Width;
        uint32_t height = m_Height;

        bool showStats = false;

        while(!m_QuitRenderThread.load())
        {
            // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
            bgfx::touch(kClearView);

            bgfx::dbgTextClear();
            bgfx::dbgTextImage(bx::max<uint16_t>(uint16_t(width / 2 / 8), 20) - 20, bx::max<uint16_t>(uint16_t(height / 2 / 16), 6) - 6, 40, 12, s_Logo, 160);
            bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats.");
            bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");
            bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
            bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");
            const bgfx::Stats* stats = bgfx::getStats();
            bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters.", stats->width, stats->height, stats->textWidth, stats->textHeight);
            // Enable stats or debug text.
            bgfx::setDebug(showStats ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);
            // Advance to next frame. Main thread will be kicked to process submitted rendering primitives.
            bgfx::frame();
        }

        bgfx::shutdown();
        return 0;
    }

    /*static*/ int32_t WindowImpl::StaticRenderThread(bx::Thread* thread, void* userData)
    {
        AFEX_ASSERT(userData != nullptr);
        return reinterpret_cast<WindowImpl*>(userData)->RenderThread(thread);
    }

    const uint8_t WindowImpl::s_Logo[4000] = {
    0xdc, 0x03, 0xdc, 0x03, 0xdc, 0x03, 0xdc, 0x03, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, // ........ . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0xdc, 0x08, //  . . . . . . ...
    0xdc, 0x03, 0xdc, 0x07, 0xdc, 0x07, 0xdc, 0x08, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, // ........ . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0xde, 0x03, 0xb0, 0x3b, 0xb1, 0x3b, 0xb2, 0x3b, 0xdb, 0x3b, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, // ...;.;.;.; . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0xdc, 0x03, 0xb1, 0x3b, 0xb2, 0x3b, //  . . . . ....;.;
    0xdb, 0x3b, 0xdf, 0x03, 0xdf, 0x3b, 0xb2, 0x3f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, // .;...;.? . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0xb1, 0x3b, 0xb1, 0x3b, 0xb2, 0x3b, 0xb2, 0x3f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  ..;.;.;.? . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0xb1, 0x3b, 0xb1, 0x3b, 0xb2, 0x3b, //  . . . . ..;.;.;
    0xb2, 0x3f, 0x20, 0x0f, 0x20, 0x0f, 0xdf, 0x03, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, // .? . ... . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0xb1, 0x3b, 0xb1, 0x3b, 0xb1, 0x3b, 0xb1, 0x3f, 0xdc, 0x0b, 0xdc, 0x03, 0xdc, 0x03, //  ..;.;.;.?......
    0xdc, 0x03, 0xdc, 0x03, 0x20, 0x0f, 0x20, 0x0f, 0xdc, 0x08, 0xdc, 0x03, 0xdc, 0x03, 0xdc, 0x03, // .... . .........
    0xdc, 0x03, 0xdc, 0x03, 0xdc, 0x03, 0xdc, 0x08, 0x20, 0x0f, 0xb1, 0x3b, 0xb1, 0x3b, 0xb1, 0x3b, // ........ ..;.;.;
    0xb1, 0x3f, 0xb1, 0x3f, 0xb2, 0x0b, 0x20, 0x0f, 0x20, 0x0f, 0xdc, 0x03, 0xdc, 0x03, 0xdc, 0x03, // .?.?.. . .......
    0x20, 0x0f, 0x20, 0x0f, 0xdc, 0x03, 0xdc, 0x03, 0xdc, 0x03, 0x20, 0x0f, 0x20, 0x01, 0x20, 0x0f, //  . ....... . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0xb2, 0x3b, 0xb1, 0x3b, 0xb0, 0x3b, 0xb0, 0x3f, 0x20, 0x0f, 0xde, 0x03, 0xb0, 0x3f, //  ..;.;.;.? ....?
    0xb1, 0x3f, 0xb2, 0x3f, 0xdd, 0x03, 0xde, 0x03, 0xdb, 0x03, 0xdb, 0x03, 0xb2, 0x3f, 0x20, 0x0f, // .?.?.........? .
    0x20, 0x0f, 0xb0, 0x3f, 0xb1, 0x3f, 0xb2, 0x3f, 0xde, 0x38, 0xb2, 0x3b, 0xb1, 0x3b, 0xb0, 0x3b, //  ..?.?.?.8.;.;.;
    0xb0, 0x3f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0xb0, 0x3b, 0xb1, 0x3b, 0xb2, 0x3b, 0xb2, 0x3f, // .? . . ..;.;.;.?
    0xdd, 0x03, 0xde, 0x03, 0xb0, 0x3f, 0xb1, 0x3f, 0xb2, 0x3f, 0xdd, 0x03, 0x20, 0x01, 0x20, 0x0f, // .....?.?.?.. . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0xb2, 0x3b, 0xb1, 0x3b, 0xb0, 0x3b, 0xb0, 0x3f, 0x20, 0x0f, 0x20, 0x0f, 0xdb, 0x03, //  ..;.;.;.? . ...
    0xb0, 0x3f, 0xb1, 0x3f, 0xdd, 0x03, 0xb1, 0x3b, 0xb0, 0x3b, 0xdb, 0x03, 0xb1, 0x3f, 0x20, 0x0f, // .?.?...;.;...? .
    0x20, 0x0f, 0x20, 0x3f, 0xb0, 0x3f, 0xb1, 0x3f, 0xb0, 0x3b, 0xb2, 0x3b, 0xb1, 0x3b, 0xb0, 0x3b, //  . ?.?.?.;.;.;.;
    0xb0, 0x3f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0xdc, 0x08, 0xdc, 0x3b, 0xb1, 0x3b, 0xb1, 0x3f, // .? . . ....;.;.?
    0xb1, 0x3b, 0xb0, 0x3b, 0xb2, 0x3b, 0xb0, 0x3f, 0xdc, 0x03, 0x20, 0x0f, 0x20, 0x01, 0x20, 0x0f, // .;.;.;.?.. . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0xb2, 0x3b, 0xb1, 0x3b, 0xb0, 0x3b, 0xb0, 0x3f, 0xdc, 0x0b, 0xdc, 0x07, 0xdb, 0x03, //  ..;.;.;.?......
    0xdb, 0x03, 0xdc, 0x38, 0x20, 0x0f, 0xdf, 0x03, 0xb1, 0x3b, 0xb0, 0x3b, 0xb0, 0x3f, 0xdc, 0x03, // ...8 ....;.;.?..
    0xdc, 0x07, 0xb0, 0x3f, 0xb1, 0x3f, 0xb2, 0x3f, 0xdd, 0x3b, 0xb2, 0x3b, 0xb1, 0x3b, 0xdc, 0x78, // ...?.?.?.;.;.;.x
    0xdf, 0x08, 0x20, 0x0f, 0x20, 0x0f, 0xde, 0x08, 0xb2, 0x3b, 0xb1, 0x3b, 0xb0, 0x3b, 0xb0, 0x3f, // .. . ....;.;.;.?
    0x20, 0x0f, 0xdf, 0x03, 0xb1, 0x3b, 0xb2, 0x3b, 0xdb, 0x03, 0xdd, 0x03, 0x20, 0x01, 0x20, 0x0f, //  ....;.;.... . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0xdc, 0x08, 0xdc, 0x08, 0xdc, 0x08, 0x20, 0x0f, //  . . . ....... .
    0x20, 0x0f, 0xb0, 0x3f, 0xb0, 0x3f, 0xb1, 0x3f, 0xdd, 0x3b, 0xdb, 0x0b, 0xdf, 0x03, 0x20, 0x0f, //  ..?.?.?.;.... .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0xdf, 0x08, 0xdf, 0x03, 0xdf, 0x03, 0xdf, 0x08, //  . . . .........
    0x20, 0x0f, 0x20, 0x0f, 0xdf, 0x08, 0xdf, 0x03, 0xdf, 0x03, 0x20, 0x0f, 0x20, 0x01, 0x20, 0x0f, //  . ....... . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0xdb, 0x08, 0xb2, 0x38, 0xb1, 0x38, 0xdc, 0x03, //  . . . ....8.8..
    0xdc, 0x07, 0xb0, 0x3b, 0xb1, 0x3b, 0xdf, 0x3b, 0xdf, 0x08, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, // ...;.;.;.. . . .
    0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, //  . . . . . . . .
    0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, //  . . . . . . . .
    0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, //  . . . . . . . .
    0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, //  . . . . . . . .
    0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, //  . . . . . . . .
    0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, //  . . . . . . . .
    0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, //  . . . . . . . .
    0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0b, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x2d, 0x08, 0x3d, 0x08, 0x20, 0x0a, 0x43, 0x0b, 0x72, 0x0b, 0x6f, 0x0b, 0x73, 0x0b, 0x73, 0x0b, // -.=. .C.r.o.s.s.
    0x2d, 0x0b, 0x70, 0x0b, 0x6c, 0x0b, 0x61, 0x0b, 0x74, 0x0b, 0x66, 0x0b, 0x6f, 0x0b, 0x72, 0x0b, // -.p.l.a.t.f.o.r.
    0x6d, 0x0b, 0x20, 0x0b, 0x72, 0x0b, 0x65, 0x0b, 0x6e, 0x0b, 0x64, 0x0b, 0x65, 0x0b, 0x72, 0x0b, // m. .r.e.n.d.e.r.
    0x69, 0x0b, 0x6e, 0x0b, 0x67, 0x0b, 0x20, 0x0b, 0x6c, 0x0b, 0x69, 0x0b, 0x62, 0x0b, 0x72, 0x0b, // i.n.g. .l.i.b.r.
    0x61, 0x0b, 0x72, 0x0b, 0x79, 0x0b, 0x20, 0x0f, 0x3d, 0x08, 0x2d, 0x08, 0x20, 0x01, 0x20, 0x0f, // a.r.y. .=.-. . .
    0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, //  . . . . . . . .
    0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, //  . . . . . . . .
    0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, //  . . . . . . . .
    0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, //  . . . . . . . .
    0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, 0x20, 0x0a, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, 0x20, 0x0f, //  . . . . . . . .
    };

}