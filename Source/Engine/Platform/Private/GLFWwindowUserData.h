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
        GLFWwindowUserData()                                        = default;
        GLFWwindowUserData(const GLFWwindowUserData&)               = default;
        GLFWwindowUserData& operator=(const GLFWwindowUserData&)    = default;
        GLFWwindowUserData(GLFWwindowUserData&& other) noexcept     = default;
        GLFWwindowUserData& operator=(GLFWwindowUserData&& other)   = default;
        ~GLFWwindowUserData()                                       = default;

        void SetCursorPIMPL(CursorImpl* pimpl) { m_CursorPIMPL = pimpl; }
        CursorImpl* GetCursorPIMPL() const { return m_CursorPIMPL; }

        void SetKeyboardPIMPL(KeyboardImpl* pimpl) { m_KeyboardPIMPL = pimpl; }
        KeyboardImpl* GetKeyboardPIMPL() { return m_KeyboardPIMPL; }

    private:
        CursorImpl* m_CursorPIMPL = nullptr;
        KeyboardImpl* m_KeyboardPIMPL = nullptr;
    };
}