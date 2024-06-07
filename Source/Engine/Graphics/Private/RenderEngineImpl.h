#pragma once

// Graphics
#include "imguiRenderer.h"

// Engine
#include <Graphics/RenderEngineArgs.h>

// External
#include <bx/handlealloc.h>
#include <bx/spscqueue.h>
#include <bx/thread.h>
#include <bx/semaphore.h>

namespace Graphics
{
    class RenderEngineImpl
    {
    public:
        explicit RenderEngineImpl(const RenderEngineArgs& args);
        ~RenderEngineImpl();

        static int32_t StaticRenderThreadFunc(bx::Thread* thread, void* userData);
        int32_t RenderThreadFunc();

        void BeginFrame() const;
        void EndFrame() const;

        void Resize(uint32_t width, uint32_t height);
        void SetDebugMode(DebugMode mode);

        bool IsValid() const { return m_Valid; }

    private:
        ImGuiRenderer m_ImguiRenderer;

        // Event queue
        bx::DefaultAllocator m_QueueAllocator;
        bx::SpScUnboundedQueue m_RenderThreadEvents;

        // Event data cache
        DebugMode m_DebugMode;

        // Lifetime management
        bx::Thread m_RenderThread;
        const RenderEngineArgs m_Args;
        bool m_Valid;

        // Synchronization
        bx::Semaphore m_InitSemaphore;
        mutable bx::Semaphore m_MainToRenderSem;
        mutable bx::Semaphore m_RenderToMainSem;

        // Stack size, if zero is passed it will use OS default thread stack
        static constexpr uint32_t k_RenderThreadStackSize = 0;

        // After this time the frame dropped
        // A value of -1 will use the bgfx default value of 5000 ms
        static constexpr int32_t k_RenderTimeoutMilliseconds = -1;

        // The shutdown timeout is similar to the render timeout but specific to
        // the final frames after we've instructed the render thread to exit.
        static constexpr int32_t k_ShutdownTimeoutMilliseconds = 1000;
    };

}