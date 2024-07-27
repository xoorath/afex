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

        // The input can be scaled if the resolution and window resolution are not the same
        // For example: if the window is 1000x1000 and we are rendering imgui at 100x100
        // we will want to scale the inputs by 0.1f so the cursor (in window coords) lines up with the graphics.
        void SetInputScale(float inputScaleX, float inputScaleY);

    private:
        Cursor& m_Cursor;
        Keyboard& m_Keyboard;

        KeyCallbackType::Token m_KeyCallbackSubscription;
        
        CursorPositionCallbackType::Token m_CursorPositionSubscription;
        CursorEnterCallbackType::Token m_CursorEnterSubscription;
        MouseButtonCallbackType::Token m_CursorButtonSubscription;
        ScrollCallbackType::Token m_CursorScrollSubscription;

        float m_InputScaleX = 1.0f;
        float m_InputScaleY = 1.0f;
    };
}