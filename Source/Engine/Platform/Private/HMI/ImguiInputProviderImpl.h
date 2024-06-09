#pragma once

// Engine
#include <Platform/HMI/Cursor.h>
#include <Platform/HMI/CursorCallbackTypes.h>
#include <Platform/HMI/Keyboard.h>
#include <Platform/HMI/KeyboardCallbackTypes.h>

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

        KeyCallbackType::Token m_KeyCallbackSubscription;
        
        CursorPositionCallbackType::Token m_CursorPositionSubscription;
        CursorEnterCallbackType::Token m_CursorEnterSubscription;
        MouseButtonCallbackType::Token m_CursorButtonSubscription;
        ScrollCallbackType::Token m_CursorScrollSubscription;
    };
}