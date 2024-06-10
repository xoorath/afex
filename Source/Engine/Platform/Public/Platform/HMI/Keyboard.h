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
        // Only the WindowImpl is allowed to create & initialize a Keyboard
        friend class WindowImpl;
        Keyboard();                                 /*=default*/
        Keyboard(const Keyboard&)                   = delete;
        Keyboard& operator=(const Keyboard&)        = delete;
        Keyboard(Keyboard&&) noexcept;              /*=default*/
        Keyboard& operator=(Keyboard&&) noexcept;   /*=default*/
        ~Keyboard();                                /*=default*/

        void Init(GLFWwindow* window);

        Core::PIMPL<KeyboardImpl, 216> m_PIMPL;
    };
}