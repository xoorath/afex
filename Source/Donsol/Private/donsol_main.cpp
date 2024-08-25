// Donsol
#include "./Gameplay/Difficulty.h"
#include "./UI/MainMenu.h"

// Engine
#include <Application/Application.h>
#include <Application/EntryPoint.h>
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Config/Config.h>
#include <Core/Logging.h>
#include <Graphics/DebugMode.h>
#include <Graphics/RenderEngine.h>
#include <Platform/HMI/Keyboard.h>
#include <Platform/Window.h>

// System
#include <string_view>
#include <memory>

using namespace std::string_view_literals;

namespace Donsol
{
    namespace SettingsKeys
    {
        // See Donsol::Difficulty::s_Difficulties for valid difficulty values
        static constexpr std::string_view Difficulty        = "donsol.difficulty"sv;

        // See Donsol::MainMenu::TrySetResolutionScale for valid resolution scale values
        static constexpr std::string_view ResolutionScale   = "donsol.resolution_scale"sv;
    };

    class DonsolApp : public Application::Application
    {
    public:
        DonsolApp()
        {
            // Use the default logging configuration
            ConfigureLogging(); 
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

            if(!InitSettings())
            {
                return false;
            }

            render.SetDebugMode(m_DebugMode);

            {
                auto onKeyEventToken = keyboard.OnKeyEvent().Add(
                    [this](Platform::KeyCode key, int32_t scanCode, Platform::KeyboardAction action, Platform::KeyboardModifier modifiers)
                    {
                        OnKeyEvent(key, scanCode, action, modifiers);
                    });
                AddShutdownProcedure("OnKeyEvent"sv,
                    [onKeyEventToken, &keyboard]()
                    {
                        keyboard.OnKeyEvent().Remove(onKeyEventToken);
                    });
            }

            {
                auto onDifficultyChangedToken = m_MainMenu.OnDifficultyChanged().Add( 
                    [this](const Donsol::Difficulty& difficulty, size_t index)
                    {
                        AFEX_UNUSED(index);
                        m_Config->SetSetting<std::string>(SettingsKeys::Difficulty, std::string(difficulty.GetName()));
                        m_Config->Save();
                    });
                AddShutdownProcedure("OnDifficultyChanged"sv,
                    [onDifficultyChangedToken, this]()
                    {
                        m_MainMenu.OnDifficultyChanged().Remove(onDifficultyChangedToken);
                    });
            }

            {
                auto onResolutionScaleChangedToken = m_MainMenu.OnResolutionScaleChanged().Add(
                    [this, &window](float scale)
                    {
                        uint32_t renderWidth, renderHeight;
                        window.GetSize(renderWidth, renderHeight);
                        SetRenderResolution(
                            static_cast<uint32_t>(static_cast<float>(renderWidth) * scale),
                            static_cast<uint32_t>(static_cast<float>(renderHeight) * scale));
                        m_Config->SetSetting<float>(SettingsKeys::ResolutionScale, scale);
                        m_Config->Save();
                    });
                AddShutdownProcedure("OnResolutionScaleChanged"sv,
                    [onResolutionScaleChangedToken, this]()
                    {
                        m_MainMenu.OnResolutionScaleChanged().Remove(onResolutionScaleChangedToken);
                    });
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
        void OnKeyEvent(Platform::KeyCode key,
                        int32_t scanCode,
                        Platform::KeyboardAction action,
                        Platform::KeyboardModifier modifiers)
        {
            Platform::Window& window = GetWindowMutable();

            AFEX_UNUSED(scanCode);
            AFEX_UNUSED(modifiers);

            if (action == Platform::KeyboardAction::Release)
            {
                switch (key)
                {
                case Platform::KeyCode::Escape:
                    window.RequestClose();
                    break;
                case Platform::KeyCode::F1:
                    CycleDebugMode();
                    break;
                }
            }
        }

        bool InitSettings()
        {
            m_Config = Core::Config::ConfigFactory("donsol"sv);
            AFEX_ASSERT_MSG(m_Config.get(), "The config factory is expected to always produce a config.");
            AddShutdownProcedure("save and close donsol config", 
                [this]()
                {
                    m_Config->Save();
                    m_Config.reset();
                });

            const std::string difficultyValue = m_Config->GetSetting<std::string>(SettingsKeys::Difficulty, "casual");
            const float resolutionScaleValue = m_Config->GetSetting<float>(SettingsKeys::ResolutionScale, 1.0);

            // Below we try to apply the values we read.
            // If they are not currently valid values we use valid values and re-write the corrected config.

            bool saveConfig = false;

            const std::string_view actualDifficulty = m_MainMenu.TrySetDifficulty(difficultyValue);
            if (difficultyValue != actualDifficulty)
            {
                m_Config->SetSetting(SettingsKeys::Difficulty, std::string(actualDifficulty));
                saveConfig = true;
            }

            const float actualResolutionScale = m_MainMenu.TrySetResolutionScale(resolutionScaleValue);
            if (resolutionScaleValue != actualResolutionScale)
            {
                m_Config->SetSetting<float>(SettingsKeys::ResolutionScale, actualResolutionScale);
                saveConfig = true;
            }

            if (saveConfig)
            {
                m_Config->Save();
            }

            return true;
        }

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

        std::shared_ptr<Core::Config> m_Config;
        Graphics::DebugMode m_DebugMode = Graphics::DebugMode::None;
        // Screens
        UI::MainMenu m_MainMenu;
    };
}
AFEX_DECLARE_ENTRY_POINT(Donsol::DonsolApp);