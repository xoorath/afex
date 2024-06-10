#pragma once

// Engine
#include <Graphics/RenderEngine.h>
#include <Graphics/RenderEngineArgs.h>

// External
#include <bgfx/bgfx.h>
#include <bx/handlealloc.h>
#include <bx/spscqueue.h>
#include <bx/thread.h>
#include <bx/semaphore.h>

// System
#include <atomic>

//////////////////////////////////////////////////////////////////////////

namespace bgfx
{
    struct Stats;
}

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    //////////////////////////////////////////////////////////////////////////
    class RenderEngineImpl
    {
    public:
        explicit RenderEngineImpl(const RenderEngineArgs& args);
        ~RenderEngineImpl();

        RenderEngineImpl()                                          = delete;
        RenderEngineImpl(const RenderEngineImpl& args)              = delete;
        RenderEngineImpl& operator=(const RenderEngineImpl& args)   = delete;
        RenderEngineImpl(RenderEngineImpl&& args)                   = delete;
        RenderEngineImpl& operator=(RenderEngineImpl&& args)        = delete;

        static int32_t StaticRenderThreadFunc(bx::Thread* thread, void* userData);
        int32_t RenderThreadFunc();

        bool IsValid() const;
        void SubmitFrame();
        void WaitForRender();
        void Resize(uint32_t width, uint32_t height);
        void GetSize(uint32_t& outWidth, uint32_t& outHeight) const;
        void SetDebugMode(DebugMode mode);
        void SetDisplayLogo(bool shouldDisplayLogo);
        RenderEngine::RenderCallback& OnRender();
        const bgfx::Stats* GetStats() const;

    private:
        void RecordSample(const bgfx::Stats* value);
        void DrawDebugText(const bgfx::Stats*) const;

        // Event queue
        bx::DefaultAllocator m_QueueAllocator;
        bx::SpScUnboundedQueue m_RenderThreadEvents;

        // Event data cache
        DebugMode m_DebugMode;
        uint32_t m_LastRequestedWidth;
        uint32_t m_LastRequestedHeight;

        // Lifetime management
        bx::Thread m_RenderThread;
        const RenderEngineArgs m_Args;
        bool m_Valid;

        // Synchronization
        bx::Semaphore m_InitSemaphore;
        bx::Semaphore m_SubmitFrameSemaphore;
        bx::Semaphore m_RenderFrameSemaphore;

        // Callbacks
        RenderEngine::RenderCallback m_OnRender;

        // misc.
        size_t m_StatsCopyIndex = 0;
        bgfx::Stats m_StatsCopy[2];
        struct
        {
            static constexpr uint32_t k_NumSamples = 100;
            int32_t m_offset = 0;
            float m_values[k_NumSamples] = { 0 };

            float m_min = 0.0f;
            float m_max = 0.0f;
            float m_avg = 0.0f;
        } m_Samples;
        bool m_showStats = true;

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