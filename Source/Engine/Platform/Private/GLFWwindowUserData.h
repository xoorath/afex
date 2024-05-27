#pragma once

// System
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class Keyboard;

    //////////////////////////////////////////////////////////////////////////
    class GLFWwindowUserData
    {
    public:
        explicit GLFWwindowUserData(Keyboard& kb)
            : m_Keyboard(kb)
        {
        }

        Keyboard& GetKeyboardMutable() { return m_Keyboard; }

    private:
        Keyboard& m_Keyboard;
    };
}