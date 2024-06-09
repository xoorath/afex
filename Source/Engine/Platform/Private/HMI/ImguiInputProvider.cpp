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
        : m_PIMPL(context, cursor, keyboard)
    {
    }
    
    /*PLATFORM_EXPORT*/ ImGuiInputProvider::ImGuiInputProvider(ImGuiInputProvider&& other) noexcept = default;
    /*PLATFORM_EXPORT*/ ImGuiInputProvider& ImGuiInputProvider::operator=(ImGuiInputProvider&& other) noexcept = default;
    /*PLATFORM_EXPORT*/ ImGuiInputProvider::~ImGuiInputProvider() = default;
}