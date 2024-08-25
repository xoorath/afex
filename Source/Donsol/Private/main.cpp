// Donsol
#include "./Gameplay/Difficulty.h"
#include "./Logging/AppLogging.h"
#include "./UI/MainMenu.h"

// Engine
#include <Application/Application.h>
#include <Application/EntryPoint.h>
#include <Core/CommonMacros.h>
#include <Core/Config/Config.h>
#include <Core/Logging.h>
#include <Graphics/DebugMode.h>
#include <Graphics/RenderEngine.h>
#include <Platform/HMI/Keyboard.h>
#include <Platform/Window.h>

// System
#include <string_view>

namespace Donsol
{
    class DonsolApp : public Application::Application
    {
    public:
        DonsolApp()
        {
            ConfigureLogging();
            AddShutdownProcedure("DonsolApp Logging", &ShutdownLogging);
        }
    protected:

        bool EarlyInit() override
        {
            return true;
        }

        bool Init() override
        {
            Platform::Window& window        = GetWindowMutable();
            Platform::Keyboard& keyboard    = window.GetKeyboardMutable();
            Graphics::RenderEngine& render  = GetRenderEngineMutable();

            using namespace std::string_view_literals;
            m_Config.emplace("donsol"sv);

            render.SetDebugMode(m_DebugMode);
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


            bool saveConfig = false;
            
            std::string difficultyValue = m_Config->GetSetting<std::string>("donsol.difficulty", "casual");
            const std::string_view actualDifficulty = m_MainMenu.TrySetDifficulty(difficultyValue);
            if(difficultyValue != actualDifficulty)
            {
                m_Config->SetSetting("donsol.difficulty", std::string(actualDifficulty));
                saveConfig = true;
            }
            m_MainMenu.OnDifficultyChanged() += 
                [this](const Donsol::Difficulty& difficulty, size_t index)
                {
                    AFEX_UNUSED(index);
                    m_Config->SetSetting<std::string>("donsol.difficulty", std::string(difficulty.GetName()));
                    m_Config->Save();
                };

            const float resolutionScaleValue = m_Config->GetSetting<float>("donsol.resolution_scale", 1.0);
            const float actualResolutionScale = m_MainMenu.TrySetResolutionScale(resolutionScaleValue);
            if(resolutionScaleValue != actualResolutionScale)
            {
                m_Config->SetSetting<float>("donsol.resolution_scale", actualResolutionScale);
                saveConfig = true;
            }
            m_MainMenu.OnResolutionScaleChanged() +=
                [this, &window](float scale)
                {
                    uint32_t renderWidth, renderHeight;
                    window.GetSize(renderWidth, renderHeight);
                    SetRenderResolution(
                        static_cast<uint32_t>(static_cast<float>(renderWidth) * scale),
                        static_cast<uint32_t>(static_cast<float>(renderHeight) * scale));
                    m_Config->SetSetting<float>("donsol.resolution_scale", scale);
                    m_Config->Save();
                };

            if(saveConfig)
            {
                m_Config->Save();
            }

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
            m_MainMenu.GUI();
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

        std::optional<Core::Config> m_Config;
        Graphics::DebugMode m_DebugMode = Graphics::DebugMode::None;
        // Screens
        UI::MainMenu m_MainMenu;
    };
}
AFEX_DECLARE_ENTRY_POINT(Donsol::DonsolApp);