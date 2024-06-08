#include <Platform/HMI/Cursor.h>

// Platform
#include "CursorImpl.h"

// Engine
#include <Core/Logging.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{

    ////////////////////////////////////////////////////////////////////////// Public
    /*explicit*/ Cursor::Cursor(GLFWwindow* window)
        : m_PIMPL(new CursorImpl(window))
    {
    }
    
    Cursor::Cursor(Cursor&& other) noexcept
        : m_PIMPL(other.m_PIMPL)
    {
        other.m_PIMPL = nullptr;
    }
    
    Cursor& Cursor::operator=(Cursor&& other) noexcept
    {
        m_PIMPL = other.m_PIMPL;
        other.m_PIMPL = nullptr;
        return *this;
    }
    
    Cursor::~Cursor()
    {
    	delete reinterpret_cast<CursorImpl*>(m_PIMPL);
    }

    /*PLATFORM_EXPORT*/ Cursor::CursorPositionCallbackType& Cursor::OnCursorPosition()
    {
        return reinterpret_cast<CursorImpl*>(m_PIMPL)->OnCursorPosition();
    }

    /*PLATFORM_EXPORT*/ Cursor::CursorEnterCallbackType& Cursor::OnCursorEnter()
    {
        return reinterpret_cast<CursorImpl*>(m_PIMPL)->OnCursorEnter();
    }

    /*PLATFORM_EXPORT*/ Cursor::CursorButtonCallbackType& Cursor::OnCursorButton()
    {
        return reinterpret_cast<CursorImpl*>(m_PIMPL)->OnCursorButton();
    }

    /*PLATFORM_EXPORT*/ Cursor::CursorScrollCallbackType& Cursor::OnCursorScroll()
    {
        return reinterpret_cast<CursorImpl*>(m_PIMPL)->OnCursorScroll();
    }

    void Cursor::GLFWcursorposfun(double xpos, double ypos)
    {
        reinterpret_cast<CursorImpl*>(m_PIMPL)->GLFWcursorposfun(xpos, ypos);
    }

    void Cursor::GLFWcursorenterfun(int entered)
    {
        reinterpret_cast<CursorImpl*>(m_PIMPL)->GLFWcursorenterfun(entered);
    }

    void Cursor::GLFWmousebuttonfun(int button, int action, int mods)
    {
        reinterpret_cast<CursorImpl*>(m_PIMPL)->GLFWmousebuttonfun(button, action, mods);
    }

    void Cursor::GLFWscrollfun(double xoffset, double yoffset)
    {
        reinterpret_cast<CursorImpl*>(m_PIMPL)->GLFWscrollfun(xoffset, yoffset);
    }
}