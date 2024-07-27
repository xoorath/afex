#pragma once

// Engine
#include <Core/Config/Config.h>
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
        ApplicationImpl();
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

        // Gets the render resolution. If the render resolution is not manually set: the
        // window resolution is used.
        void GetRenderResolution(uint32_t& outWidth, uint32_t& outHeight) const;
        // Sets the desired render resolution. This resolution will be kept even as the window resizes.
        void SetRenderResolution(uint32_t width, uint32_t height);
        // The render resolution will be set to (and kept to) the window native resolution.
        void UnsetRenderResolution();
        
    private:
        Core::Config m_Config;

        ImGuiContext* m_ImguiContext = nullptr;
        std::optional<Platform::Window> m_Window;
        std::optional<Graphics::RenderEngine> m_RenderEngine;
        std::optional<Graphics::ImGuiRenderer> m_ImguiRenderer;
        std::optional<Platform::ImGuiInputProvider> m_ImguiInputProvider;

        // during destruction this vector will be iterated in reverse and invoked.
        std::vector<std::tuple<std::string, std::function<void()>>> m_ShutdownProcedure;

        std::optional<uint32_t> m_RenderResolutionWidth;
        std::optional<uint32_t> m_RenderResolutionHeight;
    };
}