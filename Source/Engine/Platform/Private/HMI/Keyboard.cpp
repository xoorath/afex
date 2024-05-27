#include <Platform/HMI/Keyboard.h>

// Private
#include "../GLFWwindowUserData.h"

// External
#include <GLFW/glfw3.h>

namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// private static
    /*static*/ void Keyboard::GLFWkeyfunStatic(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        GLFWwindowUserData* userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
        userData->GetKeyboardMutable().GLFWkeyfun(key, scancode, action, mods);
    }

    ////////////////////////////////////////////////////////////////////////// private
    Keyboard::Keyboard(GLFWwindow* window)
        : m_Window(window)
    {
        if(m_Window != nullptr)
        {
            glfwSetKeyCallback(m_Window, &Keyboard::GLFWkeyfunStatic);
        }
    }

    Keyboard::~Keyboard()
    {
        if(m_Window != nullptr)
        {
            glfwSetKeyCallback(m_Window, nullptr);
        }
    }

    void Keyboard::GLFWkeyfun(int key, int scancode, int action, int mods)
    {
        // KeyCode key, int32_t scanCode, KeyboardAction action, int32_t modifiers
        KeyboardAction kbAction;
        switch(action)
        {
            case GLFW_PRESS: kbAction = KeyboardAction::Press; break;
            case GLFW_RELEASE: kbAction = KeyboardAction::Release; break;
            case GLFW_REPEAT: kbAction = KeyboardAction::Repeat; break;
            default: return;
        }
        m_KeyCallback(
            static_cast<KeyCode>(key),
            static_cast<int32_t>(scancode),
            kbAction,
            static_cast<int32_t>(mods));
    }
}