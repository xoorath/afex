#include <Platform/HMI/ImguiInputProvider.h>

// Platform
#include "ImguiInputProviderImpl.h"

// Engine
#include <Core/Logging.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*PLATFORM_EXPORT*/ ImGuiInputProvider::ImGuiInputProvider(ImGuiContext* context, Cursor& cursor, Keyboard& keyboard)
        : m_PIMPL(new ImGuiInputProviderImpl(context, cursor, keyboard))
    {
    }
    
    /*PLATFORM_EXPORT*/ ImGuiInputProvider::ImGuiInputProvider(ImGuiInputProvider&& other) noexcept
        : m_PIMPL(other.m_PIMPL)
    {
        other.m_PIMPL = nullptr;
    }
    
    /*PLATFORM_EXPORT*/ ImGuiInputProvider& ImGuiInputProvider::operator=(ImGuiInputProvider&& other) noexcept
    {
        m_PIMPL = other.m_PIMPL;
        other.m_PIMPL = nullptr;
        return *this;
    }
    
    /*PLATFORM_EXPORT*/ ImGuiInputProvider::~ImGuiInputProvider()
    {
    	delete reinterpret_cast<ImGuiInputProviderImpl*>(m_PIMPL);
    }
}