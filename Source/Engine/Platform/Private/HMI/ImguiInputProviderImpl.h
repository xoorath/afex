#pragma once

// Engine
#include <Platform/HMI/Cursor.h>
#include <Platform/HMI/Keyboard.h>

// System
#include <array>
#include <cstdint>

//////////////////////////////////////////////////////////////////////////
struct ImGuiContext;
typedef struct GLFWwindow GLFWwindow;

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////
    class ImGuiInputProviderImpl
    {
    public:
        ImGuiInputProviderImpl(ImGuiContext* context, Cursor& cursor, Keyboard& keyboard);
        ~ImGuiInputProviderImpl();

        ImGuiInputProviderImpl()                                                = default;
        ImGuiInputProviderImpl(const ImGuiInputProviderImpl&)                   = default;
        ImGuiInputProviderImpl(ImGuiInputProviderImpl&&) noexcept               = default;
        ImGuiInputProviderImpl& operator=(const ImGuiInputProviderImpl&)        = default;
        ImGuiInputProviderImpl& operator=(ImGuiInputProviderImpl&&) noexcept    = default;
        
    private:
        Cursor& m_Cursor;
        Keyboard& m_Keyboard;

        Keyboard::KeyCallbackType::Token m_KeyCallbackSubscription;
        
        Cursor::CursorPositionCallbackType::Token m_CursorPositionSubscription;
        Cursor::CursorEnterCallbackType::Token m_CursorEnterSubscription;
        Cursor::CursorButtonCallbackType::Token m_CursorButtonSubscription;
        Cursor::CursorScrollCallbackType::Token m_CursorScrollSubscription;
    };
}