#include "ImguiInputProviderImpl.h"

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>

// External
#include <bgfx/bgfx.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

// System
#include <array>


//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// internal
    class ImGuiKeymap
    {
    public:
        ImGuiKeymap()
        {
            m_KeyMap[static_cast<size_t>(KeyCode::Space)] = ImGuiKey::ImGuiKey_Space;
            m_KeyMap[static_cast<size_t>(KeyCode::Apostrophe)] = ImGuiKey::ImGuiKey_Apostrophe;
            m_KeyMap[static_cast<size_t>(KeyCode::Comma)] = ImGuiKey::ImGuiKey_Comma;
            m_KeyMap[static_cast<size_t>(KeyCode::Minus)] = ImGuiKey::ImGuiKey_Minus;
            m_KeyMap[static_cast<size_t>(KeyCode::Period)] = ImGuiKey::ImGuiKey_Period;
            m_KeyMap[static_cast<size_t>(KeyCode::Slash)] = ImGuiKey::ImGuiKey_Slash;
            m_KeyMap[static_cast<size_t>(KeyCode::Num0)] = ImGuiKey::ImGuiKey_0;
            m_KeyMap[static_cast<size_t>(KeyCode::Num1)] = ImGuiKey::ImGuiKey_1;
            m_KeyMap[static_cast<size_t>(KeyCode::Num2)] = ImGuiKey::ImGuiKey_2;
            m_KeyMap[static_cast<size_t>(KeyCode::Num3)] = ImGuiKey::ImGuiKey_3;
            m_KeyMap[static_cast<size_t>(KeyCode::Num4)] = ImGuiKey::ImGuiKey_4;
            m_KeyMap[static_cast<size_t>(KeyCode::Num5)] = ImGuiKey::ImGuiKey_5;
            m_KeyMap[static_cast<size_t>(KeyCode::Num6)] = ImGuiKey::ImGuiKey_6;
            m_KeyMap[static_cast<size_t>(KeyCode::Num7)] = ImGuiKey::ImGuiKey_7;
            m_KeyMap[static_cast<size_t>(KeyCode::Num8)] = ImGuiKey::ImGuiKey_8;
            m_KeyMap[static_cast<size_t>(KeyCode::Num9)] = ImGuiKey::ImGuiKey_9;
            m_KeyMap[static_cast<size_t>(KeyCode::Semicolon)] = ImGuiKey::ImGuiKey_Semicolon;
            m_KeyMap[static_cast<size_t>(KeyCode::Equal)] = ImGuiKey::ImGuiKey_Equal;
            m_KeyMap[static_cast<size_t>(KeyCode::A)] = ImGuiKey::ImGuiKey_A;
            m_KeyMap[static_cast<size_t>(KeyCode::B)] = ImGuiKey::ImGuiKey_B;
            m_KeyMap[static_cast<size_t>(KeyCode::C)] = ImGuiKey::ImGuiKey_C;
            m_KeyMap[static_cast<size_t>(KeyCode::D)] = ImGuiKey::ImGuiKey_D;
            m_KeyMap[static_cast<size_t>(KeyCode::E)] = ImGuiKey::ImGuiKey_E;
            m_KeyMap[static_cast<size_t>(KeyCode::F)] = ImGuiKey::ImGuiKey_F;
            m_KeyMap[static_cast<size_t>(KeyCode::G)] = ImGuiKey::ImGuiKey_G;
            m_KeyMap[static_cast<size_t>(KeyCode::H)] = ImGuiKey::ImGuiKey_H;
            m_KeyMap[static_cast<size_t>(KeyCode::I)] = ImGuiKey::ImGuiKey_I;
            m_KeyMap[static_cast<size_t>(KeyCode::J)] = ImGuiKey::ImGuiKey_J;
            m_KeyMap[static_cast<size_t>(KeyCode::K)] = ImGuiKey::ImGuiKey_K;
            m_KeyMap[static_cast<size_t>(KeyCode::L)] = ImGuiKey::ImGuiKey_L;
            m_KeyMap[static_cast<size_t>(KeyCode::M)] = ImGuiKey::ImGuiKey_M;
            m_KeyMap[static_cast<size_t>(KeyCode::N)] = ImGuiKey::ImGuiKey_N;
            m_KeyMap[static_cast<size_t>(KeyCode::O)] = ImGuiKey::ImGuiKey_O;
            m_KeyMap[static_cast<size_t>(KeyCode::P)] = ImGuiKey::ImGuiKey_P;
            m_KeyMap[static_cast<size_t>(KeyCode::Q)] = ImGuiKey::ImGuiKey_Q;
            m_KeyMap[static_cast<size_t>(KeyCode::R)] = ImGuiKey::ImGuiKey_R;
            m_KeyMap[static_cast<size_t>(KeyCode::S)] = ImGuiKey::ImGuiKey_S;
            m_KeyMap[static_cast<size_t>(KeyCode::T)] = ImGuiKey::ImGuiKey_T;
            m_KeyMap[static_cast<size_t>(KeyCode::U)] = ImGuiKey::ImGuiKey_U;
            m_KeyMap[static_cast<size_t>(KeyCode::V)] = ImGuiKey::ImGuiKey_V;
            m_KeyMap[static_cast<size_t>(KeyCode::W)] = ImGuiKey::ImGuiKey_W;
            m_KeyMap[static_cast<size_t>(KeyCode::X)] = ImGuiKey::ImGuiKey_X;
            m_KeyMap[static_cast<size_t>(KeyCode::Y)] = ImGuiKey::ImGuiKey_Y;
            m_KeyMap[static_cast<size_t>(KeyCode::Z)] = ImGuiKey::ImGuiKey_Z;
            m_KeyMap[static_cast<size_t>(KeyCode::LeftBracket)] = ImGuiKey::ImGuiKey_LeftBracket;
            m_KeyMap[static_cast<size_t>(KeyCode::Backslash)] = ImGuiKey::ImGuiKey_Backslash;
            m_KeyMap[static_cast<size_t>(KeyCode::RightBracket)] = ImGuiKey::ImGuiKey_RightBracket;
            m_KeyMap[static_cast<size_t>(KeyCode::GraveAccent)] = ImGuiKey::ImGuiKey_GraveAccent;
            m_KeyMap[static_cast<size_t>(KeyCode::World1)] = ImGuiKey::ImGuiKey_COUNT;
            m_KeyMap[static_cast<size_t>(KeyCode::World2)] = ImGuiKey::ImGuiKey_COUNT;
            m_KeyMap[static_cast<size_t>(KeyCode::Escape)] = ImGuiKey::ImGuiKey_Escape;
            m_KeyMap[static_cast<size_t>(KeyCode::Enter)] = ImGuiKey::ImGuiKey_Enter;
            m_KeyMap[static_cast<size_t>(KeyCode::Tab)] = ImGuiKey::ImGuiKey_Tab;
            m_KeyMap[static_cast<size_t>(KeyCode::Backspace)] = ImGuiKey::ImGuiKey_Backspace;
            m_KeyMap[static_cast<size_t>(KeyCode::Insert)] = ImGuiKey::ImGuiKey_Insert;
            m_KeyMap[static_cast<size_t>(KeyCode::Delete)] = ImGuiKey::ImGuiKey_Delete;
            m_KeyMap[static_cast<size_t>(KeyCode::Right)] = ImGuiKey::ImGuiKey_RightArrow;
            m_KeyMap[static_cast<size_t>(KeyCode::Left)] = ImGuiKey::ImGuiKey_LeftArrow;
            m_KeyMap[static_cast<size_t>(KeyCode::Down)] = ImGuiKey::ImGuiKey_DownArrow;
            m_KeyMap[static_cast<size_t>(KeyCode::Up)] = ImGuiKey::ImGuiKey_UpArrow;
            m_KeyMap[static_cast<size_t>(KeyCode::PageUp)] = ImGuiKey::ImGuiKey_PageUp;
            m_KeyMap[static_cast<size_t>(KeyCode::PageDown)] = ImGuiKey::ImGuiKey_PageDown;
            m_KeyMap[static_cast<size_t>(KeyCode::Home)] = ImGuiKey::ImGuiKey_Home;
            m_KeyMap[static_cast<size_t>(KeyCode::End)] = ImGuiKey::ImGuiKey_End;
            m_KeyMap[static_cast<size_t>(KeyCode::CapsLock)] = ImGuiKey::ImGuiKey_CapsLock;
            m_KeyMap[static_cast<size_t>(KeyCode::ScrollLock)] = ImGuiKey::ImGuiKey_ScrollLock;
            m_KeyMap[static_cast<size_t>(KeyCode::NumLock)] = ImGuiKey::ImGuiKey_NumLock;
            m_KeyMap[static_cast<size_t>(KeyCode::PrintScreen)] = ImGuiKey::ImGuiKey_PrintScreen;
            m_KeyMap[static_cast<size_t>(KeyCode::Pause)] = ImGuiKey::ImGuiKey_Pause;
            m_KeyMap[static_cast<size_t>(KeyCode::F1)] = ImGuiKey::ImGuiKey_F1;
            m_KeyMap[static_cast<size_t>(KeyCode::F2)] = ImGuiKey::ImGuiKey_F2;
            m_KeyMap[static_cast<size_t>(KeyCode::F3)] = ImGuiKey::ImGuiKey_F3;
            m_KeyMap[static_cast<size_t>(KeyCode::F4)] = ImGuiKey::ImGuiKey_F4;
            m_KeyMap[static_cast<size_t>(KeyCode::F5)] = ImGuiKey::ImGuiKey_F5;
            m_KeyMap[static_cast<size_t>(KeyCode::F6)] = ImGuiKey::ImGuiKey_F6;
            m_KeyMap[static_cast<size_t>(KeyCode::F7)] = ImGuiKey::ImGuiKey_F7;
            m_KeyMap[static_cast<size_t>(KeyCode::F8)] = ImGuiKey::ImGuiKey_F8;
            m_KeyMap[static_cast<size_t>(KeyCode::F9)] = ImGuiKey::ImGuiKey_F9;
            m_KeyMap[static_cast<size_t>(KeyCode::F10)] = ImGuiKey::ImGuiKey_F10;
            m_KeyMap[static_cast<size_t>(KeyCode::F11)] = ImGuiKey::ImGuiKey_F11;
            m_KeyMap[static_cast<size_t>(KeyCode::F12)] = ImGuiKey::ImGuiKey_F12;
            m_KeyMap[static_cast<size_t>(KeyCode::F13)] = ImGuiKey::ImGuiKey_F13;
            m_KeyMap[static_cast<size_t>(KeyCode::F14)] = ImGuiKey::ImGuiKey_F14;
            m_KeyMap[static_cast<size_t>(KeyCode::F15)] = ImGuiKey::ImGuiKey_F15;
            m_KeyMap[static_cast<size_t>(KeyCode::F16)] = ImGuiKey::ImGuiKey_F16;
            m_KeyMap[static_cast<size_t>(KeyCode::F17)] = ImGuiKey::ImGuiKey_F17;
            m_KeyMap[static_cast<size_t>(KeyCode::F18)] = ImGuiKey::ImGuiKey_F18;
            m_KeyMap[static_cast<size_t>(KeyCode::F19)] = ImGuiKey::ImGuiKey_F19;
            m_KeyMap[static_cast<size_t>(KeyCode::F20)] = ImGuiKey::ImGuiKey_F20;
            m_KeyMap[static_cast<size_t>(KeyCode::F21)] = ImGuiKey::ImGuiKey_F21;
            m_KeyMap[static_cast<size_t>(KeyCode::F22)] = ImGuiKey::ImGuiKey_F22;
            m_KeyMap[static_cast<size_t>(KeyCode::F23)] = ImGuiKey::ImGuiKey_F23;
            m_KeyMap[static_cast<size_t>(KeyCode::F24)] = ImGuiKey::ImGuiKey_F24;
            m_KeyMap[static_cast<size_t>(KeyCode::F25)] = ImGuiKey::ImGuiKey_COUNT;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp0)] = ImGuiKey::ImGuiKey_Keypad0;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp1)] = ImGuiKey::ImGuiKey_Keypad1;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp2)] = ImGuiKey::ImGuiKey_Keypad2;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp3)] = ImGuiKey::ImGuiKey_Keypad3;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp4)] = ImGuiKey::ImGuiKey_Keypad4;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp5)] = ImGuiKey::ImGuiKey_Keypad5;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp6)] = ImGuiKey::ImGuiKey_Keypad6;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp7)] = ImGuiKey::ImGuiKey_Keypad7;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp8)] = ImGuiKey::ImGuiKey_Keypad8;
            m_KeyMap[static_cast<size_t>(KeyCode::Kp9)] = ImGuiKey::ImGuiKey_Keypad9;
            m_KeyMap[static_cast<size_t>(KeyCode::KpDecimal)] = ImGuiKey::ImGuiKey_KeypadDecimal;
            m_KeyMap[static_cast<size_t>(KeyCode::KpDivide)] = ImGuiKey::ImGuiKey_KeypadDivide;
            m_KeyMap[static_cast<size_t>(KeyCode::KpMultiply)] = ImGuiKey::ImGuiKey_KeypadMultiply;
            m_KeyMap[static_cast<size_t>(KeyCode::KpSubtract)] = ImGuiKey::ImGuiKey_KeypadSubtract;
            m_KeyMap[static_cast<size_t>(KeyCode::KpAdd)] = ImGuiKey::ImGuiKey_KeypadAdd;
            m_KeyMap[static_cast<size_t>(KeyCode::KpEnter)] = ImGuiKey::ImGuiKey_KeypadEnter;
            m_KeyMap[static_cast<size_t>(KeyCode::KpEqual)] = ImGuiKey::ImGuiKey_KeypadEqual;
            m_KeyMap[static_cast<size_t>(KeyCode::LeftShift)] = ImGuiKey::ImGuiKey_LeftShift;
            m_KeyMap[static_cast<size_t>(KeyCode::LeftControl)] = ImGuiKey::ImGuiKey_LeftCtrl;
            m_KeyMap[static_cast<size_t>(KeyCode::LeftAlt)] = ImGuiKey::ImGuiKey_LeftAlt;
            m_KeyMap[static_cast<size_t>(KeyCode::LeftSuper)] = ImGuiKey::ImGuiKey_LeftSuper;
            m_KeyMap[static_cast<size_t>(KeyCode::RightShift)] = ImGuiKey::ImGuiKey_RightShift;
            m_KeyMap[static_cast<size_t>(KeyCode::RightControl)] = ImGuiKey::ImGuiKey_RightCtrl;
            m_KeyMap[static_cast<size_t>(KeyCode::RightAlt)] = ImGuiKey::ImGuiKey_RightAlt;
            m_KeyMap[static_cast<size_t>(KeyCode::RightSuper)] = ImGuiKey::ImGuiKey_RightSuper;
            m_KeyMap[static_cast<size_t>(KeyCode::Menu)] = ImGuiKey::ImGuiKey_Menu;
            m_KeyMap[static_cast<size_t>(KeyCode::Last)] = ImGuiKey::ImGuiKey_COUNT;
        }

        ImGuiKey operator[](KeyCode index) const
        {
            return m_KeyMap[static_cast<size_t>(index)];
        }

        ImGuiKeymap(const ImGuiKeymap&)                             = delete;
        ImGuiKeymap(ImGuiKeymap&&) noexcept                         = delete;
        ImGuiKeymap& operator=(const ImGuiKeymap&)      = delete;
        ImGuiKeymap& operator=(ImGuiKeymap&&) noexcept  = delete;
        ~ImGuiKeymap()                                              = default;

    private:
        std::array<ImGuiKey, static_cast<size_t>(KeyCode::Last)+1> m_KeyMap;
    };

    static ImGuiKeymap g_ImGuiKeymap;

    ////////////////////////////////////////////////////////////////////////// Public
    ImGuiInputProviderImpl::ImGuiInputProviderImpl(ImGuiContext* context,Cursor& cursor, Keyboard& keyboard)
        : m_Cursor(cursor)
        , m_Keyboard(keyboard)
    {
        AFEX_ASSERT(context != nullptr);

        m_KeyCallbackSubscription = keyboard.OnKeyEvent().Add(
            [](KeyCode key, int32_t scanCode, KeyboardAction action, KeyboardModifier modifiers)
            {
                ImGuiIO& io = ImGui::GetIO();
                AFEX_UNUSED(scanCode);
                AFEX_UNUSED(modifiers);

                switch (action)
                {
                case KeyboardAction::Press:
                    io.AddKeyEvent(g_ImGuiKeymap[key], true);
                    break;
                case KeyboardAction::Release:
                    io.AddKeyEvent(g_ImGuiKeymap[key], false);
                    break;
                }
            });

        m_CursorPositionSubscription = cursor.OnCursorPosition().Add(
            [](float xPos, float yPos)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(xPos, yPos);
            });

        m_CursorButtonSubscription = cursor.OnCursorButton().Add(
            [](int button, int action, int mods)
            {
                AFEX_UNUSED(mods);
                ImGuiIO& io = ImGui::GetIO();
                switch(button)
                {
                    case GLFW_MOUSE_BUTTON_LEFT:
                        io.AddMouseButtonEvent(ImGuiMouseButton_Left, action == GLFW_PRESS || action == GLFW_REPEAT);
                        break;
                    case GLFW_MOUSE_BUTTON_RIGHT:
                        io.AddMouseButtonEvent(ImGuiMouseButton_Right, action == GLFW_PRESS || action == GLFW_REPEAT);
                        break;
                }
            });
    }

    ImGuiInputProviderImpl::~ImGuiInputProviderImpl()
    {
        m_Keyboard.OnKeyEvent().Remove(m_KeyCallbackSubscription);
        m_Cursor.OnCursorPosition().Remove(m_CursorPositionSubscription);
        m_Cursor.OnCursorButton().Remove(m_CursorButtonSubscription);
    }
}