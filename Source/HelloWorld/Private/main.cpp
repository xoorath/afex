// Hello World
#include "./Logging/AppLogging.h"

// Engine
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>
#include <Platform/Window.h>
#include <Graphics/RenderEngine.h>
#include <Graphics/RenderEngineArgs.h>

// System
#include <cinttypes>
#include <memory.h>
#include <string_view>

// External
#include <imgui.h>

int main()
{
    // Configure SPD first. This will let the logging macros from Core/Logging output consistently. 
    HelloWorld::ConfigureLogging();

    AFEX_LOG_INFO("Welcome to spdlog from main!");

    if (!Platform::Window::GlobalInit())
    {
        HelloWorld::ShutdownLogging();
        return -100;
    }

    constexpr uint32_t k_Width = 1920;
    constexpr uint32_t k_Height = 1080;
    ImGuiContext* imguiContext = ImGui::CreateContext();
    if (imguiContext == nullptr)
    {
        AFEX_LOG_ERROR("Failed to create imgui context");
        ImGui::DestroyContext(imguiContext);
        HelloWorld::ShutdownLogging();
        return -200;
    }

    std::unique_ptr<Platform::Window> appWindow;
    {
        Platform::WindowArgs windowArgs(
            /*title=*/  "HelloWorld",
            /*width=*/  k_Width,
            /*height=*/ k_Height);
        appWindow = std::make_unique<Platform::Window>(windowArgs);
        if (!appWindow->IsValid())
        {
            Platform::Window::GlobalShutdown();
            ImGui::DestroyContext(imguiContext);
            HelloWorld::ShutdownLogging();
            return -200;
        }
    }
    std::unique_ptr<Graphics::RenderEngine> renderEngine;
    {
        Graphics::RenderEngineArgs renderArgs(
            /*nativeWindowHandle=*/ appWindow->GetNativeWindowHandle(),
            /*imguiContext=*/       imguiContext,
            /*width=*/              k_Width,
            /*height=*/             k_Height);
        renderEngine = std::make_unique<Graphics::RenderEngine>(renderArgs);
        if (!renderEngine->IsValid())
        {
            appWindow.reset();
            Platform::Window::GlobalShutdown();
            ImGui::DestroyContext(imguiContext);
            HelloWorld::ShutdownLogging();
            return -300;
        }
    }

    appWindow->GetKeyboardMutable().OnKeyEvent() +=
        [window = appWindow.get()](Platform::KeyCode key, int32_t scanCode, Platform::KeyboardAction action, int32_t modifiers)
        {
            AFEX_UNUSED(scanCode);
            AFEX_UNUSED(modifiers);

            if (action == Platform::KeyboardAction::Release
                && key == Platform::KeyCode::Escape)
            {
                window->RequestClose();
            }
        };

    while (!appWindow->CloseRequested())
    {
        appWindow->PollEvents();
        renderEngine->BeginFrame();
        {
            ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiCond_Appearing);
            ImGui::Begin("HelloWorld");

            ImGui::End();
        }
        renderEngine->EndFrame();
    }

    appWindow.reset();
    Platform::Window::GlobalShutdown();
    ImGui::DestroyContext(imguiContext);
    HelloWorld::ShutdownLogging();
}
