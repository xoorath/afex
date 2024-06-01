// Hello World
#include "./Logging/AppLogging.h"

// Engine
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>
#include <Platform/Window.h>

// System
#include <cinttypes>
#include <memory.h>

int main() {
    // Configure SPD first. This will let the logging macros from Core/Logging output consistently. 
    HelloWorld::ConfigureLogging();

    AFEX_LOG_INFO("Welcome to spdlog from main!");

    if(Platform::Window::GlobalInit())
    {
        std::unique_ptr<Platform::Window> appWindow = std::make_unique<Platform::Window>(1920, 1080, "Hello World");
        if(appWindow->IsValid())
        {
            appWindow->MakeWindowContextCurrent();
            appWindow->GetKeyboardMutable().OnKeyEvent() += 
                [window=appWindow.get()](Platform::KeyCode key, int32_t scanCode, Platform::KeyboardAction action, int32_t modifiers)
                {
                    AFEX_UNUSED(scanCode);
                    AFEX_UNUSED(modifiers);

                    if(action == Platform::KeyboardAction::Release 
                        && key == Platform::KeyCode::Escape)
                    {
                        window->RequestClose();
                    }
                };

            while(!appWindow->CloseRequested())
            {
                appWindow->Clear();
                // render frame

                appWindow->SwapBuffers();
                appWindow->PollEvents();
            }
        }
        appWindow.reset();
        Platform::Window::GlobalShutdown();
    }
    HelloWorld::ShutdownLogging();
}
