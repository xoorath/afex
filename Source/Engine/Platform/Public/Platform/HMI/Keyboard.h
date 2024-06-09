#pragma once

#include <Core/PIMPL.h>
#include <Platform/HMI/KeyboardCallbackTypes.h>
#include <Platform/Platform.export.h>

//////////////////////////////////////////////////////////////////////////

typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    class WindowImpl;
    class KeyboardImpl;
    //////////////////////////////////////////////////////////////////////////
    class Keyboard
    {
    public:
        PLATFORM_EXPORT KeyCallbackType& OnKeyEvent();
        PLATFORM_EXPORT CharCallbackType& OnCharEvent();

    /*internal:*/
        KeyboardImpl* GetPIMPL() noexcept; // Exposed to be provided via glfw user data
    private:
        // Only the WindowImpl is allowed to create a Keyboard
        friend class WindowImpl;
        explicit Keyboard(GLFWwindow* window);

        Keyboard(const Keyboard&)               = delete;
        Keyboard(Keyboard&&)                    = delete;
        Keyboard& operator = (const Keyboard&)  = delete;
        Keyboard& operator = (Keyboard&&)       = delete;
        ~Keyboard(); /*=default*/

        Core::PIMPL<KeyboardImpl, 216> m_PIMPL;
    };
}