// Hello World
#include "./Logging/AppLogging.h"

// Engine
#include <Core/Logging.h>
#include <Core/Window.h>

// System
#include <memory.h>
#include <conio.h>

int main() {
    // Configure SPD first. This will let the logging macros from Core/Logging output consistently. 
    HelloWorld::ConfigureLogging();

    AFEX_LOG_INFO("Welcome to spdlog from main!");

    if(Platform::Window::GlobalInit())
    {
        std::unique_ptr<Platform::Window> appWindow = std::make_unique<Platform::Window>(640, 480, "Hello World");
        if(appWindow->IsValid())
        {
            appWindow->MakeWindowContextCurrent();
            while(!appWindow->CloseRequested())
            {
                appWindow->Clear();
                // render frame

                appWindow->SwapBuffers();
                appWindow->PollEvents();
            }
        }
        Platform::Window::GlobalShutdown();
    }
    HelloWorld::ShutdownLogging();
}
