#pragma once

#include <Platform/HMI/KeyboardCallbackTypes.h>

//////////////////////////////////////////////////////////////////////////

typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////
    class KeyboardImpl
    {
    public:
        explicit KeyboardImpl(GLFWwindow* window);
        ~KeyboardImpl();
        
        KeyboardImpl()                                      = delete;
        KeyboardImpl(const KeyboardImpl&)                   = delete;
        KeyboardImpl(KeyboardImpl&&) noexcept               = delete;
        KeyboardImpl& operator=(const KeyboardImpl&)        = delete;
        KeyboardImpl& operator=(KeyboardImpl&&) noexcept    = delete;

        KeyCallbackType& OnKeyEvent();
        CharCallbackType& OnCharEvent();

    private:
        void GLFWkeyfun(int key, int scancode, int action, int mods);

        GLFWwindow* m_Window;
        KeyCallbackType m_KeyCallback;
        CharCallbackType m_CharCallback;
    };
}