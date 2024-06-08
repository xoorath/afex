#pragma once

// System
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class Cursor;
    class Keyboard;

    //////////////////////////////////////////////////////////////////////////
    class GLFWwindowUserData
    {
    public:
        GLFWwindowUserData(Cursor& cursor, Keyboard& kb)
            : m_Cursor(cursor)
            , m_Keyboard(kb)
        {
        }

        Cursor& GetCursorMutable() { return m_Cursor; }
        Keyboard& GetKeyboardMutable() { return m_Keyboard; }

    private:
        Cursor& m_Cursor;
        Keyboard& m_Keyboard;
    };
}