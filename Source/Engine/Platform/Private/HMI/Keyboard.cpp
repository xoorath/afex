#include <Platform/HMI/Keyboard.h>

// Platform
#include "../GLFWwindowUserData.h"
#include "KeyboardImpl.h"

// Engine
#include <Core/Assert.h>

// External
#include <GLFW/glfw3.h>

namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// Private
    Keyboard::Keyboard(GLFWwindow* window)
    {
        m_PIMPL.emplace(window);
    }

    Keyboard::~Keyboard() = default;

    ////////////////////////////////////////////////////////////////////////// Public
    /*PLATFORM_EXPORT*/ KeyCallbackType& Keyboard::OnKeyEvent()
    {
        return m_PIMPL->OnKeyEvent();
    }
    
    /*PLATFORM_EXPORT*/ CharCallbackType& Keyboard::OnCharEvent()
    {
        return m_PIMPL->OnCharEvent();
    }

    ////////////////////////////////////////////////////////////////////////// Internal
    KeyboardImpl* Keyboard::GetPIMPL() noexcept
    {
        return m_PIMPL.GetMutable();
    }
}