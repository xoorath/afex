#pragma once

// Platform
#include "GLFWwindowUserData.h"

// Engine
#include <Platform/HMI/Cursor.h>
#include <Platform/HMI/Keyboard.h>
#include <Platform/Window.h>

// System
#include <cstdint>

//////////////////////////////////////////////////////////////////////////

typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////
    class WindowImpl
    {

    public:
        explicit WindowImpl(const WindowArgs& args);
        WindowImpl()                                = delete;
        WindowImpl(const WindowImpl&)               = delete;
        WindowImpl& operator=(const WindowImpl&)    = delete;
        WindowImpl(WindowImpl&&)                    = delete;
        WindowImpl& operator=(WindowImpl&&)         = delete;
        ~WindowImpl();
        bool IsValid() const;

        static void ErrorCallback(int error_code, const char* description);
        static bool GlobalInit();
        static void GlobalShutdown();

        void RequestClose();
        bool CloseRequested() const;
        void PollEvents();

        const Keyboard& GetKeyboard() const;
        Keyboard& GetKeyboardMutable();
        const Cursor& GetCursor() const;
        Cursor& GetCursorMutable();

        Window::ResizeCallbackType& OnResize();
        void GetSize(uint32_t& outWidth, uint32_t& outHeight) const;

        void* GetNativeWindowHandle() const;
    private:
        // GLFW
        GLFWwindow* m_Window = nullptr;

        // Engine
        Cursor m_Cursor;
        Keyboard m_Keyboard;
        GLFWwindowUserData m_GLFWwindowUserData;

        // Callbacks
        Window::ResizeCallbackType m_ResizeCallback;
        uint32_t m_PreviousWidth = 0;
        uint32_t m_PreviousHeight = 0;
    };
}