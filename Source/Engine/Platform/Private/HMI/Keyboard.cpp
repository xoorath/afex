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

    ////////////////////////////////////////////////////////////////////////// Private
    Keyboard::Keyboard()                                = default;
    Keyboard::Keyboard(Keyboard&&) noexcept             = default;
    Keyboard& Keyboard::operator=(Keyboard&&) noexcept  = default;
    Keyboard::~Keyboard()                               = default;

    void Keyboard::Init(GLFWwindow* window)
    {
        m_PIMPL.emplace(window);
    }
}