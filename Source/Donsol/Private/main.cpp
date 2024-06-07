// Donsol
#include "./Logging/AppLogging.h"

// Engine
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>
#include <Platform/Window.h>
#include <Platform/WindowArgs.h>
#include <Graphics/RenderEngineArgs.h>
#include <Graphics/RenderEngine.h>

// System
#include <cinttypes>
#include <memory.h>
#include <string_view>

// External
#include <imgui.h>

static const char* k_AppName = "Donsol";
class DonsolApp
{
public:
    DonsolApp()
    {
        // Configure SPD first. This will let the logging macros from Core/Logging output consistently. 
        Donsol::ConfigureLogging();
        m_ShutdownProcedure.push_front(&Donsol::ShutdownLogging);

        AFEX_LOG_INFO(__FUNCTION__ "()");

        if (Platform::Window::GlobalInit())
        {
            m_ShutdownProcedure.push_front(&Platform::Window::GlobalShutdown);
        }
        else
        {
            m_ErrorCode = -100;
            return;
        }

        m_ImguiContext = ImGui::CreateContext();
        if (m_ImguiContext)
        {
            m_ShutdownProcedure.push_front([ctx = m_ImguiContext]() { ImGui::DestroyContext(ctx); });
        }
        else
        {
            AFEX_LOG_ERROR("Failed to create imgui context");
            m_ErrorCode = -200;
            return;
        }

        {
            Platform::WindowArgs windowArgs(
                /*title=*/          k_AppName,
                /*imguiContext=*/   m_ImguiContext,
                /*width=*/          k_Width,
                /*height=*/         k_Height);
            m_Window = std::make_unique<Platform::Window>(windowArgs);
            if (m_Window->IsValid())
            {
                m_ShutdownProcedure.push_front([this]() { m_Window.reset(); });
            }
            else
            {
                m_ErrorCode = -300;
                return;
            }
        }

        {
            Graphics::RenderEngineArgs renderArgs(
                /*nativeWindowHandle=*/ m_Window->GetNativeWindowHandle(),
                /*imguiContext=*/       m_ImguiContext,
                /*width=*/              k_Width,
                /*height=*/             k_Height);
            m_RenderEngine = std::make_unique<Graphics::RenderEngine>(renderArgs);
            if (m_RenderEngine->IsValid())
            {
                m_ShutdownProcedure.push_front([this]() { m_RenderEngine.reset(); });
            }
            else
            {
                m_ErrorCode = -400;
                return;
            }
        }


        m_Window->OnResize().Add(
            [renderEngine = m_RenderEngine.get()](uint32_t width, uint32_t height)
            {
                renderEngine->Resize(width, height);
            });

        m_Window->GetKeyboardMutable().OnKeyEvent() +=
            [window = m_Window.get()](Platform::KeyCode key, int32_t scanCode, Platform::KeyboardAction action, int32_t modifiers)
            {
                AFEX_UNUSED(scanCode);
                AFEX_UNUSED(modifiers);

                if (action == Platform::KeyboardAction::Release
                    && key == Platform::KeyCode::Escape)
                {
                    window->RequestClose();
                }
            };
    }

    ~DonsolApp()
    {
        AFEX_LOG_INFO(__FUNCTION__ "()");
        for (auto proc : m_ShutdownProcedure)
        {
            proc();
        }
    }

    int32_t GetErrorCode() const { return m_ErrorCode; }

    int32_t Run()
    {
        while (!m_Window->CloseRequested())
        {
            m_Window->PollEvents();
            m_RenderEngine->BeginFrame();
            {
                ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiCond_Appearing);
                ImGui::Begin(k_AppName);

                ImGui::End();
            }
            m_RenderEngine->EndFrame();
        }
        return 0;
    }

private:
    static constexpr uint32_t k_Width = 1920;
    static constexpr uint32_t k_Height = 1080;

    ImGuiContext* m_ImguiContext = nullptr;
    std::unique_ptr<Platform::Window> m_Window;
    std::unique_ptr<Graphics::RenderEngine> m_RenderEngine;

    std::list<std::function<void()>> m_ShutdownProcedure;

    int32_t m_ErrorCode = 0;
};

int main()
{
    DonsolApp app;

    if (app.GetErrorCode() != 0)
    {
        return app.GetErrorCode();
    }

    return app.Run();
}
