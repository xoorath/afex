#include <Platform/Window.h>

// Platform
#include "WindowImpl.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>
#include <Platform/WindowArgs.h>
#include <Platform/HMI/Cursor.h>
#include <Platform/HMI/Keyboard.h>

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
        : m_PIMPL()
    {
        m_PIMPL.emplace(args);
        AFEX_LOG_TRACE("Window created {} ({}, {}, {}) ",
            (IsValid() ? "successfully" : "unsuccessfully"), 
            args.GetWidth(), 
            args.GetHeight(), 
            args.GetTitle().c_str());
    }

    /*PLATFORM_EXPORT*/ Window::~Window()
    {
        AFEX_LOG_TRACE("Window destroyed");
    }

    /*PLATFORM_EXPORT*/ bool Window::IsValid() const
    {
        return m_PIMPL->IsValid();
    }

    /*PLATFORM_EXPORT*/ void Window::RequestClose()
    {
        m_PIMPL->RequestClose();
    }

    /*PLATFORM_EXPORT*/ bool Window::CloseRequested() const
    {
        return m_PIMPL->CloseRequested();
    }

    /*PLATFORM_EXPORT*/ void Window::PollEvents()
    {
        m_PIMPL->PollEvents();
    }

    /*PLATFORM_EXPORT*/ const Keyboard& Window::GetKeyboard() const
    {
        return m_PIMPL->GetKeyboard();
    }

    /*PLATFORM_EXPORT*/ Keyboard& Window::GetKeyboardMutable()
    {
        return m_PIMPL->GetKeyboardMutable();
    }

    /*PLATFORM_EXPORT*/ const Cursor& Window::GetCursor() const
    {
        return m_PIMPL->GetCursor();
    }

    /*PLATFORM_EXPORT*/ Cursor& Window::GetCursorMutable()
    {
        return m_PIMPL->GetCursorMutable();
    }

    /*PLATFORM_EXPORT*/ Window::ResizeCallbackType& Window::OnResize()
    {
        return m_PIMPL->OnResize();
    }

    /*PLATFORM_EXPORT*/ void Window::GetSize(uint32_t& outWidth, uint32_t& outHeight) const
    {
        return m_PIMPL->GetSize(outWidth, outHeight);
    }

    /*PLATFORM_EXPORT*/ void* Window::GetNativeWindowHandle() const
    {
        return m_PIMPL->GetNativeWindowHandle();
    }
}