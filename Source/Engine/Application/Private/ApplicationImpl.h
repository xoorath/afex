#pragma once

// Engine
#include <Platform/HMI/ImguiInputProvider.h>
#include <Platform/Window.h>
#include <Graphics/ImguiRenderer.h>
#include <Graphics/RenderEngine.h>

// System
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <tuple>
#include <vector>

//////////////////////////////////////////////////////////////////////////

struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Application
{
    //////////////////////////////////////////////////////////////////////////
    class ApplicationImpl
    {
    public:
        ApplicationImpl()                                           = default;
        ~ApplicationImpl();
        ApplicationImpl(const ApplicationImpl&)                     = delete;
        ApplicationImpl(ApplicationImpl&&) noexcept                 = delete;
        ApplicationImpl& operator=(const ApplicationImpl&)          = delete;
        ApplicationImpl& operator=(ApplicationImpl&&) noexcept      = delete;

        bool Init();

        bool CloseRequested() const;
        void PollEvents();
        void BeginGUI();
        void EndGUI();
        void SubmitFrame();
        void WaitForRenderer();

        void AddShutdownProcedure(std::string_view debugName, std::function<void()> procedure);

        const Graphics::RenderEngine& GetRenderEngine() const { return m_RenderEngine.value(); }
        Graphics::RenderEngine& GetRenderEngineMutable() { return m_RenderEngine.value(); }

        const Platform::Window& GetWindow() const { return m_Window.value(); }
        Platform::Window& GetWindowMutable() { return m_Window.value(); }

        float GetRenderScale() const;
        void SetRenderScale(float scale);
        
    private:
        // Future configurable values:
        struct
        {
            uint32_t GetWidth() const { return 1920; }
            uint32_t GetHeight() const { return 1080; }
            std::string_view GetTitle() const { return "AFEX"; }
        } m_Config;

        ImGuiContext* m_ImguiContext = nullptr;
        std::optional<Platform::Window> m_Window;
        std::optional<Graphics::RenderEngine> m_RenderEngine;
        std::optional<Graphics::ImGuiRenderer> m_ImguiRenderer;
        std::optional<Platform::ImGuiInputProvider> m_ImguiInputProvider;

        // during destruction this vector will be iterated in reverse and invoked.
        std::vector<std::tuple<std::string, std::function<void()>>> m_ShutdownProcedure;

        float m_RenderScale = 1.0f;
    };
}