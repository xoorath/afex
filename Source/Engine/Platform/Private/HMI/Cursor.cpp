#include <Platform/HMI/Cursor.h>

// Platform
#include "CursorImpl.h"

// Engine
#include <Core/Logging.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*PLATFORM_EXPORT*/ CursorPositionCallbackType& Cursor::OnCursorPosition()
    {
        return m_PIMPL->OnCursorPosition();
    }

    /*PLATFORM_EXPORT*/ CursorEnterCallbackType& Cursor::OnCursorEnter()
    {
        return m_PIMPL->OnCursorEnter();
    }

    /*PLATFORM_EXPORT*/ MouseButtonCallbackType& Cursor::OnMouseButton()
    {
        return m_PIMPL->OnMouseButton();
    }

    /*PLATFORM_EXPORT*/ ScrollCallbackType& Cursor::OnScroll()
    {
        return m_PIMPL->OnScroll();
    }

    ////////////////////////////////////////////////////////////////////////// Internal
    CursorImpl* Cursor::GetPIMPL()
    {
        return m_PIMPL.GetMutable();
    }

    ////////////////////////////////////////////////////////////////////////// Private
    /*explicit*/ Cursor::Cursor()                       = default;
    Cursor::Cursor(Cursor&& other) noexcept             = default;
    Cursor& Cursor::operator=(Cursor&& other) noexcept  = default;
    Cursor::~Cursor()                                   = default;

    void Cursor::Init(GLFWwindow* window)
    {
        m_PIMPL.emplace(window);
    }
}