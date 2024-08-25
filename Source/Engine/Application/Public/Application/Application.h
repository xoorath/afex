#pragma once

// Engine
#include <Core/PIMPL.h>
#include <Application/Application.export.h>

// System
#include <functional>
#include <string_view>

namespace Core
{
    class Filesystem;
}

namespace Graphics
{
    class RenderEngine;
}

namespace Platform
{
    class Window;
}

//////////////////////////////////////////////////////////////////////////
namespace Application
{
    class ApplicationImpl;

    //////////////////////////////////////////////////////////////////////////
    // Application lifecycle:
    //
    // EARLY INIT
    // 1 The engine will first perform it's on early init to setup the 
    //   filesystem, filesystem paths, and to load the engine config.
    // 
    // 2 The application will then get a chance to EarlyInit. This can be used
    //   to configure the engine before engine systems come online.
    //
    // INIT
    // 1 The engine systems will be initialized.
    //
    // 2 The application init will then be called
    // 
    // UPDATE AND RENDER
    // 1 The render function is called. This is where render calls may be 
    //   submitted for later display.
    //
    // 2 The GUI function is called.
    //
    // 3 The render calls from the first two steps are submitted. The GPU
    //   begins drawing the frame.
    //
    // 4 The update function is called. This runs concurrently with rendering
    //   happening in the background.
    //
    // 5 The main thread waits for the renderer to complete.
    //
    // SHUTDOWN
    // The application can be terminated early on during initialization if
    // engine systems fail to startup or if the application returns false from
    // any initialization sequence. When the application finishes running the
    // application may be destroyed. During destruction all shutdown procedures
    // are run in reverse chronological order from when they were added with
    // AddShutdownProcedure.
    class Application
    {
    public:
        APPLICATION_EXPORT Application();
        APPLICATION_EXPORT ~Application(); /*= default*/
        APPLICATION_EXPORT int32_t Run(int argc, const char* argv[]);

    protected:
        Application(const Application&)                 = delete;
        Application(Application&&) noexcept             = delete;
        Application& operator=(const Application&)      = delete;
        Application& operator=(Application&&) noexcept  = delete;

        // Return true if initialization was successful and the app may be run.
        // Runs before the engine systems are initialized.
        virtual bool EarlyInit() = 0;

        // Return true if initialization was successful and the app may be run.
        // Runs after the engine is initialized.
        virtual bool Init() = 0;

        // Update app logic. Runs concurrently with the render thread.
        virtual void Update() = 0;

        // Prepare and submit render calls to the render thread.
        virtual void Render() = 0;

        // Update ImGui
        virtual void GUI() = 0;

        // Shutdown procedures are run in reverse order when the application shuts down.
        // It's recommended to add shutdown code after successful initialization of systems
        // this will ensure they are torn down later. Example:
        // 
        // if(InitGameSystem())
        // {
        //      AddShutdownProcedure("Game System Name", &ShutdownGameSystem);
        // }
        // else
        // {
        //     return false; // GameSystem failed to init.
        // }
        APPLICATION_EXPORT void AddShutdownProcedure(std::string_view debugName, std::function<void()> procedure);
        
        //////////////////////////////////////////////////////////////////////////
        // Engine systems:
        // These systems are expected to be valid by the time Init is called.

        APPLICATION_EXPORT const Core::Filesystem& GetFilesystem() const;

        APPLICATION_EXPORT const Graphics::RenderEngine& GetRenderEngine() const;
        APPLICATION_EXPORT Graphics::RenderEngine& GetRenderEngineMutable();

        APPLICATION_EXPORT const Platform::Window& GetWindow() const;
        APPLICATION_EXPORT Platform::Window& GetWindowMutable();


        //////////////////////////////////////////////////////////////////////////
        // Config:

        APPLICATION_EXPORT void GetRenderResolution(uint32_t& outWidth, uint32_t& outHeight) const;
        // The render resolution will default to the window size unless overridden here.
        APPLICATION_EXPORT void SetRenderResolution(uint32_t width, uint32_t height);


        //////////////////////////////////////////////////////////////////////////
        // Default configurations:
        //  It's expected that applications may wish to use these configuration
        //  functions when being first developed and then fine tune these default
        //  configs later

        APPLICATION_EXPORT void ConfigureLogging();
    private:
        Core::PIMPL<ApplicationImpl, 3248> m_PIMPL;
    };
}