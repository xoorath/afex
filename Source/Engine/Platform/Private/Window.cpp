#include <Platform/Window.h>

// Platform
#include "WindowImpl.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>
#include <Platform/WindowArgs.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// public static

    /*PLATFORM_EXPORT static*/ bool Window::GlobalInit()
    {
        return WindowImpl::GlobalInit();
    }

    /*PLATFORM_EXPORT static*/ void Window::GlobalShutdown()
    {
        WindowImpl::GlobalShutdown();
    }

    ////////////////////////////////////////////////////////////////////////// public

    /*PLATFORM_EXPORT explicit*/ Window::Window(const WindowArgs& args)
    {
        m_PIMPL = reinterpret_cast<void*>(WindowImpl::Create(args));
        AFEX_LOG_TRACE("Window created {} ({}, {}, {}) ",
            (IsValid() ? "successfully" : "unsuccessfully"), 
            args.GetWidth(), 
            args.GetHeight(), 
            args.GetTitle().c_str());
    }

    /*PLATFORM_EXPORT*/ Window::~Window()
    {
        AFEX_LOG_TRACE("Window destroyed");
        delete reinterpret_cast<WindowImpl*>(m_PIMPL);
    }

    /*PLATFORM_EXPORT*/ void Window::RequestClose()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->RequestClose();
    }

    /*PLATFORM_EXPORT*/ bool Window::CloseRequested() const
    {
        return reinterpret_cast<const WindowImpl*>(m_PIMPL)->CloseRequested();
    }

    /*PLATFORM_EXPORT*/ void Window::PollEvents()
    {
        reinterpret_cast<WindowImpl*>(m_PIMPL)->PollEvents();
    }

    /*PLATFORM_EXPORT*/ const Keyboard& Window::GetKeyboard() const
    {
        return reinterpret_cast<const WindowImpl*>(m_PIMPL)->GetKeyboard();
    }

    /*PLATFORM_EXPORT*/ Keyboard& Window::GetKeyboardMutable()
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->GetKeyboardMutable();
    }

    /*PLATFORM_EXPORT*/ Keyboard::KeyCallbackType& Window::OnKeyEvent()
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->OnKeyEvent();
    }

    /*PLATFORM_EXPORT*/ Keyboard::CharCallbackType& Window::OnCharEvent()
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->OnCharEvent();
    }

    /*PLATFORM_EXPORT*/ Window::ResizeCallbackType& Window::OnResize()
    {
        return reinterpret_cast<WindowImpl*>(m_PIMPL)->OnResize();
    }

    /*PLATFORM_EXPORT*/ void* Window::GetNativeWindowHandle() const
    {
        return reinterpret_cast<const WindowImpl*>(m_PIMPL)->GetNativeWindowHandle();
    }

    
}