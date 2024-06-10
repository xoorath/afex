#include "MainMenu.h"

// Donsol
#include "../Gameplay/Difficulty.h"

// Engine
#include <Core/Logging.h>

// External
#include <imgui.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////
namespace Donsol::UI
{
    ////////////////////////////////////////////////////////////////////////// Public
    MainMenu::MainMenu()
    {

        for(size_t i = 0; i < Difficulty::GetDifficultyCount(); ++i)
        {
            m_DifficultyOptions.append(Difficulty::GetDifficulty(i).GetName());
            m_DifficultyOptions.append(1, '\0');
        }
        m_DifficultyOptions.append(1, '\0');
    }

    void MainMenu::GUI()
    {
        switch(m_State)
        {
        default:
        case State::Menu:
            GUI_Menu();
            break;
        case State::Play:
            GUI_Play();
            break;
        case State::Settings:
            GUI_Settings();
            break;
        }
    }

    MainMenu::DifficultyChangedDelegate& MainMenu::OnDifficultyChanged()
    {
        return m_OnDifficultyChanged;
    }

    MainMenu::ResolutionScaleChangedDelegate& MainMenu::OnResolutionScaleChanged()
    {
        return m_OnResolutionScaleChanged;
    }

    ////////////////////////////////////////////////////////////////////////// Private
    void MainMenu::GUI_Menu()
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin(
            "Main Menu",
            nullptr,
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoScrollbar
        );

        
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 buttonSize = ImVec2(200, 50);

        ImGui::SetCursorPosX((windowSize.x - buttonSize.x) * 0.5f);
        ImGui::SetCursorPosY((windowSize.y - buttonSize.y) * 0.4f);
        if (ImGui::Button("Play", buttonSize)) {
            m_State = State::Play;
        }

        ImGui::SetCursorPosX((windowSize.x - buttonSize.x) * 0.5f);
        ImGui::SetCursorPosY((windowSize.y - buttonSize.y) * 0.6f);
        if (ImGui::Button("Settings", buttonSize)) {
            m_State = State::Settings;
        }

        ImGui::End();
    }

    void MainMenu::GUI_Play()
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin(
            "Play",
            nullptr,
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoScrollbar
        );

        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 buttonSize = ImVec2(200, 50);

        ImGui::SetCursorPosX((windowSize.x - buttonSize.x) * 0.5f);
        ImGui::SetCursorPosY((windowSize.y - buttonSize.y) * 0.5f);
        if (ImGui::Button("Main Menu", buttonSize)) {
            m_State = State::Menu;
        }

        ImGui::End();
    }

    void MainMenu::GUI_Settings()
    {
        const float windowWidth = 400.0f; // Width of the settings window
        const float windowHeight = 350.0f; // Height of the settings window
        const ImVec2 windowSize = ImVec2(windowWidth, windowHeight);

        const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        const ImVec2 windowPos = ImVec2((displaySize.x - windowWidth) * 0.5f, (displaySize.y - windowHeight) * 0.5f);

        const float elementWidth = windowWidth - 100.0f;
        const float buttonWidth = 100.0f;
        const float buttonHeight = 30.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 20));

        ImGui::SetNextWindowPos(windowPos);
        ImGui::SetNextWindowSize(windowSize);

        ImGui::Begin(
            "Settings",
            nullptr,
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoScrollbar
        );

        const char* resolutionScale = 
            "1x\0"
            "2x\0"
            "3x\0"
            "\0" ;

        ImGui::Spacing();
        ImGui::SetCursorPosX((windowSize.x - elementWidth) * 0.5f);
        ImGui::Text("Graphics");

        ImGui::SetCursorPosX((windowSize.x - elementWidth) * 0.5f);
        ImGui::PushItemWidth(elementWidth - 100.0f);
        int32_t resolutionScaleIndex = m_ResolutionScaleIndex;
        ImGui::Combo("Scale", &resolutionScaleIndex, resolutionScale);
        SetResolutionScaleIndex(resolutionScaleIndex);

        ImGui::PopItemWidth();

        ImGui::SetCursorPosX((windowSize.x - elementWidth) * 0.5f);
        ImGui::Checkbox("Fullscreen", &m_Fullscreen);

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::SetCursorPosX((windowSize.x - elementWidth) * 0.5f);
        ImGui::Text("Gameplay");

        ImGui::SetCursorPosX((windowSize.x - elementWidth) * 0.5f);
        ImGui::PushItemWidth(elementWidth - 100.0f);
        int32_t difficulty = m_DifficultyIndex;
        ImGui::Combo("Difficulty", &difficulty, m_DifficultyOptions.c_str());
        SetDifficultyIndex(difficulty);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
        if (ImGui::Button("Back", ImVec2(buttonWidth, buttonHeight))) {
            m_State = State::Menu;
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }

    void MainMenu::SetDifficultyIndex(int32_t index)
    {
        if(m_DifficultyIndex != index)
        {
            m_DifficultyIndex = index;
            auto& dif = Difficulty::GetDifficulty(static_cast<size_t>(index));
            m_OnDifficultyChanged.Invoke(dif, static_cast<size_t>(index));
        }
    }

    void MainMenu::SetResolutionScaleIndex(int32_t index)
    {
        if(m_ResolutionScaleIndex != index)
        {
            m_ResolutionScaleIndex = index;
            switch (m_ResolutionScaleIndex)
            {
            default:
            case 0: m_OnResolutionScaleChanged.Invoke(1.0f); break;
            case 1: m_OnResolutionScaleChanged.Invoke(2.0f); break;
            case 2: m_OnResolutionScaleChanged.Invoke(3.0f); break;
            }
        }
    }
}