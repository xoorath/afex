#include "KeyboardImpl.h"

// Platform
#include "../GLFWwindowUserData.h"

// Engine
#include <Core/Logging.h>

// External
#include <GLFW/glfw3.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// Public
    KeyboardImpl::KeyboardImpl(GLFWwindow* window)
        : m_Window(window)
        , m_KeyCallback()
        , m_CharCallback()
    {
        if (m_Window != nullptr)
        {
            glfwSetKeyCallback(m_Window,
                [](GLFWwindow* window, int key, int scancode, int action, int mods)
                {
                    auto userData = reinterpret_cast<GLFWwindowUserData*>(glfwGetWindowUserPointer(window));
                    auto self = reinterpret_cast<KeyboardImpl*>(userData->GetKeyboardPIMPL());
                    self->GLFWkeyfun(key, scancode, action, mods);
                });
        }
    }

    KeyboardImpl::~KeyboardImpl()
    {
        if (m_Window != nullptr)
        {
            glfwSetKeyCallback(m_Window, nullptr);
        }
    }

    KeyCallbackType& KeyboardImpl::OnKeyEvent()
    {
        return m_KeyCallback;
    }

    CharCallbackType& KeyboardImpl::OnCharEvent()
    {
        return m_CharCallback;
    }

    ////////////////////////////////////////////////////////////////////////// Private

    void KeyboardImpl::GLFWkeyfun(int key, int scancode, int action, int mods)
    {
        KeyboardAction kbAction;
        switch (action)
        {
        case GLFW_PRESS: kbAction = KeyboardAction::Press; break;
        case GLFW_RELEASE: kbAction = KeyboardAction::Release; break;
        case GLFW_REPEAT: kbAction = KeyboardAction::Repeat; break;
        default: return;
        }
        m_KeyCallback.Invoke(
            static_cast<KeyCode>(key),
            static_cast<int32_t>(scancode),
            kbAction,
            static_cast<KeyboardModifier>(mods));
    }
}