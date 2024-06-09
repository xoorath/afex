#pragma once

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class KeyboardImpl;
    class CursorImpl;
    //////////////////////////////////////////////////////////////////////////
    class GLFWwindowUserData
    {
    public:
        GLFWwindowUserData(CursorImpl* cursorPIMPL, KeyboardImpl* keyboardPIMPL)
            : m_CursorPIMPL(cursorPIMPL)
            , m_KeyboardPIMPL(keyboardPIMPL)
        {
        }

        CursorImpl* GetCursorPIMPL() const { return m_CursorPIMPL; }
        KeyboardImpl* GetKeyboardPIMPL() { return m_KeyboardPIMPL; }

        GLFWwindowUserData(GLFWwindowUserData&& other) noexcept
            : m_CursorPIMPL(other.m_CursorPIMPL)
            , m_KeyboardPIMPL(other.m_KeyboardPIMPL)
        {
            other.m_CursorPIMPL = nullptr;
            other.m_KeyboardPIMPL = nullptr;
        }

        GLFWwindowUserData& operator=(GLFWwindowUserData&& other) noexcept
        {
            m_CursorPIMPL = other.m_CursorPIMPL;
            m_KeyboardPIMPL = other.m_KeyboardPIMPL;
            other.m_CursorPIMPL = nullptr;
            other.m_KeyboardPIMPL = nullptr;
            return *this;
        }

        GLFWwindowUserData()                                        = delete;
        GLFWwindowUserData(const GLFWwindowUserData&)               = delete;
        GLFWwindowUserData& operator=(const GLFWwindowUserData&)    = delete;
        ~GLFWwindowUserData()                                       = default;

    private:
        CursorImpl* m_CursorPIMPL;
        KeyboardImpl* m_KeyboardPIMPL;
    };
}