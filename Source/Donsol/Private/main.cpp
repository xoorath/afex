// Engine
#include <Application/Application.h>
#include <Application/EntryPoint.h>
#include <Core/CommonMacros.h>
#include <Graphics/DebugMode.h>
#include <Graphics/RenderEngine.h>
#include <Platform/HMI/Keyboard.h>
#include <Platform/Window.h>

#include "./Logging/AppLogging.h"

class DonsolApp : public Application::Application
{
protected:
    bool EarlyInit() override
    {
        Donsol::ConfigureLogging();
        AddShutdownProcedure(&Donsol::ShutdownLogging);

        return true;
    }


    bool Init() override
    {
        Platform::Window& window = GetWindowMutable();
        Platform::Keyboard& keyboard = window.GetKeyboardMutable();
        Graphics::RenderEngine& render = GetRenderEngineMutable();

        keyboard.OnKeyEvent() +=
            [this, &window, &render](Platform::KeyCode key, int32_t scanCode, Platform::KeyboardAction action, Platform::KeyboardModifier modifiers)
            {
                AFEX_UNUSED(scanCode);
                AFEX_UNUSED(modifiers);

                if (action == Platform::KeyboardAction::Release)
                {
                    switch(key)
                    {
                        case Platform::KeyCode::Escape:
                            window.RequestClose();
                            break;
                        case Platform::KeyCode::F1:
                            CycleDebugMode();
                            break;
                    }
                }
            };
        return true;
    }


    void Update() override
    {
        
    }


    void Render() override
    {
    }


    void GUI() override
    {
    }

private:
    void CycleDebugMode()
    {
        Graphics::RenderEngine& render = GetRenderEngineMutable();
        switch(m_DebugMode)
        {
            case Graphics::DebugMode::None:
                m_DebugMode = Graphics::DebugMode::Text;
                break;
            case Graphics::DebugMode::Text:
                m_DebugMode = Graphics::DebugMode::Stats;
                break;
            default:
            case Graphics::DebugMode::Stats:
                m_DebugMode = Graphics::DebugMode::None;
                break;
        }
        render.SetDebugMode(m_DebugMode);
    }

    Graphics::DebugMode m_DebugMode = Graphics::DebugMode::None;
};

AFEX_DECLARE_ENTRY_POINT(DonsolApp);