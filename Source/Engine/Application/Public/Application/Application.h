#pragma once

// Engine
#include <Core/PIMPL.h>
#include <Application/Application.export.h>

// System
#include <functional>
#include <string_view>

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
    class Application
    {
    public:
        APPLICATION_EXPORT Application();
        APPLICATION_EXPORT ~Application(); /*= default*/
        APPLICATION_EXPORT int32_t Run();

    protected:
        Application(const Application&)                 = delete;
        Application(Application&&) noexcept             = delete;
        Application& operator=(const Application&)      = delete;
        Application& operator=(Application&&) noexcept  = delete;

        // Return true if initialization was successful and the app may be run.
        // Runs before the engine is initialized.
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
        //      AddShutdownProcedure(&ShutdownGameSystem);
        // }
        // else
        // {
        //     return false; // GameSystem failed to init.
        // }
        APPLICATION_EXPORT void AddShutdownProcedure(std::string_view debugName, std::function<void()> procedure);
        
        //////////////////////////////////////////////////////////////////////////
        // Engine systems:
        // These systems are expected to be valid by the time Init is called.

        APPLICATION_EXPORT const Graphics::RenderEngine& GetRenderEngine() const;
        APPLICATION_EXPORT Graphics::RenderEngine& GetRenderEngineMutable();

        APPLICATION_EXPORT const Platform::Window& GetWindow() const;
        APPLICATION_EXPORT Platform::Window& GetWindowMutable();

        //////////////////////////////////////////////////////////////////////////
        // Settings:
        APPLICATION_EXPORT float GetRenderScale() const;
        APPLICATION_EXPORT void SetRenderScale(float scale);

    
    private:
        Core::PIMPL<ApplicationImpl, 3152> m_PIMPL;
    };
}