#pragma once

// Platform
#include "GLFWwindowUserData.h"

// Engine
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
        explicit WindowImpl(GLFWwindow* window);

        ~WindowImpl();

        static WindowImpl* Create(const WindowArgs& args);
        static void ErrorCallback(int error_code, const char* description);
        static bool GlobalInit();
        static void GlobalShutdown();

        void RequestClose();
        bool CloseRequested() const;
        void PollEvents();

        const Keyboard& GetKeyboard() const;
        Keyboard& GetKeyboardMutable();

        Keyboard::KeyCallbackType& OnKeyEvent();
        Keyboard::CharCallbackType& OnCharEvent();
        Window::ResizeCallbackType& OnResize();

        void* GetNativeWindowHandle() const;
    private:
        // GLFW
        GLFWwindow* m_Window = nullptr;

        // Engine
        Keyboard m_Keyboard;
        GLFWwindowUserData m_GLFWwindowUserData;

        // Callbacks
        Window::ResizeCallbackType m_ResizeCallback;
        uint32_t m_PreviousWidth = 0;
        uint32_t m_PreviousHeight = 0;
    };
}