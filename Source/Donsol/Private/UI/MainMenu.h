#pragma once

// Engine
#include <Core/Signal.h>

// System
#include <cstdint>
#include <string>

//////////////////////////////////////////////////////////////////////////

namespace Donsol
{
    class Difficulty;
}

//////////////////////////////////////////////////////////////////////////
namespace Donsol::UI
{
    //////////////////////////////////////////////////////////////////////////
    class MainMenu
    {
    public:
        using DifficultyChangedDelegate = Core::Signal<void(const Difficulty& difficulty, size_t index)>;
        using ResolutionScaleChangedDelegate = Core::Signal<void(float scale)>;

        MainMenu();
        MainMenu(const MainMenu&)                   = default;
        MainMenu(MainMenu&&) noexcept               = default;
        MainMenu& operator=(const MainMenu&)        = default;
        MainMenu& operator=(MainMenu&&) noexcept    = default;
        ~MainMenu()                                 = default;

        void GUI();

        DifficultyChangedDelegate& OnDifficultyChanged();
        ResolutionScaleChangedDelegate& OnResolutionScaleChanged();

        // Tries to set the difficulty by name (case insensitive)
        // The name of the difficulty set will be returned, using a default difficulty
        // if the difficulty provided could not be found
        const std::string_view TrySetDifficulty(std::string_view value);
        // Tries to set the resolution scale. If the value is not one of the supported resolution scales
        // the closest value will be set instead and the value set will be returned.
        float TrySetResolutionScale(float value);

    private:
        void GUI_Menu();
        void GUI_Play();
        void GUI_Config();

        void SetDifficultyIndex(int32_t index);
        void SetResolutionScaleIndex(int32_t index);

        enum class State
        {
            Menu,
            Play,
            Config
        };
        State m_State = State::Menu;

        bool m_Fullscreen = false;

        int32_t m_DifficultyIndex = 0;
        std::string m_DifficultyOptions;

        int32_t m_ResolutionScaleIndex = 0;
        float m_ResolutionScale = 1.0f;

        DifficultyChangedDelegate m_OnDifficultyChanged;
        ResolutionScaleChangedDelegate m_OnResolutionScaleChanged;
    };
}