#include "RenderEngineImpl.h"

// Graphics
#include "bgfxCallbacks.h"
#include "Event.h"
#include "Logo.h"

// Engine
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>
#include <Graphics/DebugMode.h>
#include <Graphics/RenderEngineArgs.h>
#include <Graphics/ViewId.h>

// External
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/string.h>

// System
#include <atomic>
#include <limits>

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    ////////////////////////////////////////////////////////////////////////// internal
    /*explicit*/ RenderEngineImpl::RenderEngineImpl(const RenderEngineArgs& args)
        : m_QueueAllocator()
        , m_RenderThreadEvents(&m_QueueAllocator)
        , m_DebugMode(DebugMode::Default)
        , m_RenderThread()
        , m_Args(args)
        , m_Valid(false)
        , m_InitSemaphore()
        , m_SubmitFrameSemaphore()
        , m_OnRender()
        , m_StatsCopyIndex(0)
        , m_StatsCopy { 0 }
    {
        // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
        // Most graphics APIs must be used on the same thread that created the window.
        bgfx::renderFrame();


        if (!m_RenderThread.init(
            /*fn=*/         &RenderEngineImpl::StaticRenderThreadFunc,
            /*userData=*/   reinterpret_cast<void*>(this),
            /*stackSize=*/  k_RenderThreadStackSize,
            /*name=*/       "AFEX Render Thread"
        ))
        {
            AFEX_LOG_ERROR("Failed to create the render thread");
            return;
        }

        bgfx::renderFrame();

        // pump frames until initialization is done.
        // bgfx init requires some amount of frames - I am unsure the architectural reason why
        while (!m_InitSemaphore.wait(1))
        {
            bgfx::renderFrame();
        }

        m_Valid = true;
    }

    RenderEngineImpl::~RenderEngineImpl()
    {
        using namespace std::chrono_literals;
        if (m_RenderThread.isRunning())
        {
            m_RenderThreadEvents.push(new ExitEvent());
            while (bgfx::RenderFrame::NoContext != bgfx::renderFrame(k_ShutdownTimeoutMilliseconds))
            {
                m_SubmitFrameSemaphore.post();
            }
            m_RenderThread.shutdown();
            AFEX_ASSERT_MSG(m_RenderThread.getExitCode() == 0, "Render thread shutdown was not successful");
            AFEX_LOG_TRACE("Render thread stopped. Exit code: {}", m_RenderThread.getExitCode());
        }
    }

    bool RenderEngineImpl::IsValid() const
    {
        return m_Valid;
    }

    void RenderEngineImpl::SubmitFrame()
    {
        bgfx::renderFrame(k_RenderTimeoutMilliseconds);
        m_SubmitFrameSemaphore.post();
    }

    void RenderEngineImpl::WaitForRender()
    {
        m_RenderFrameSemaphore.wait();
    }

    /*static*/ int32_t RenderEngineImpl::StaticRenderThreadFunc(bx::Thread* thread, void* userData)
    {
        AFEX_UNUSED(thread);
        return reinterpret_cast<RenderEngineImpl*>(userData)->RenderThreadFunc();
    }

    int32_t RenderEngineImpl::RenderThreadFunc()
    {
        bgfx::Init init;
        init.platformData.nwh = m_Args.GetNativeWindowHandle();
        init.resolution.width = m_Args.GetWidth();
        init.resolution.height = m_Args.GetHeight();
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.callback = new Callbacks();

        if (!bgfx::init(init))
        {
            AFEX_LOG_ERROR("(RenderThread) bgfx::init failed");
            return -1;
        }

        {
            const bgfx::Stats* initialStats = bgfx::getStats();
            bx::memCopy(&m_StatsCopy[0], initialStats, sizeof(m_StatsCopy[0]));
            bx::memCopy(&m_StatsCopy[1], initialStats, sizeof(m_StatsCopy[0]));

            // clear the encoder stats for now since we don't have space reserved for them.
            m_StatsCopy[0].numEncoders = 0;
            m_StatsCopy[0].encoderStats = nullptr;
            m_StatsCopy[1].numEncoders = 0;
            m_StatsCopy[1].encoderStats = nullptr;

            for(uint32_t i = 0; i < m_Samples.k_NumSamples; ++i)
            {
                RecordSample(initialStats);
            }
        }

        m_InitSemaphore.post();

        bgfx::setViewClear(Graphics::ViewId::k_Clear, BGFX_CLEAR_COLOR, 0x1a1111ff);
        bgfx::setViewRect(Graphics::ViewId::k_Clear, 0, 0, bgfx::BackbufferRatio::Equal);

        uint32_t width = m_Args.GetWidth();
        uint32_t height = m_Args.GetHeight();

        // font size in pixels
        const uint32_t k_DbgTextWidth = 8;
        const uint32_t k_DbgTextHeight = 16;

        uint32_t renderFrameNumber = 0;

        bool exit = false;
        while (!exit)
        {
            // This cast to EventType is relying on the fact that all valid message types
            // have an EventType member as the first member. See Event.h for more info.
            while (EventType* ev = reinterpret_cast<EventType*>(m_RenderThreadEvents.pop()))
            {
                switch (*ev)
                {
                case EventType::Exit:
                {
                    ExitEvent* exitEvent = reinterpret_cast<ExitEvent*>(ev);
                    exit = true;
                    delete exitEvent;
                    break;
                }
                case EventType::Resize:
                {
                    ResizeEvent* resizeEvent = reinterpret_cast<ResizeEvent*>(ev);
                    bgfx::reset(resizeEvent->GetWidth(), resizeEvent->GetHeight(), BGFX_RESET_VSYNC);
                    bgfx::setViewRect(Graphics::ViewId::k_Clear, 0, 0, bgfx::BackbufferRatio::Equal);
                    width = resizeEvent->GetWidth();
                    height = resizeEvent->GetHeight();
                    delete resizeEvent;
                    break;
                }
                case EventType::DebugMode:
                {
                    DebugModeEvent* debugModeEvent = reinterpret_cast<DebugModeEvent*>(ev);
                    m_DebugMode = debugModeEvent->GetDebugMode();
                    delete debugModeEvent;
                    break;
                }
                default:
                {
                    delete ev;
                    break;
                }
                }
            }

            bgfx::touch(Graphics::ViewId::k_Clear);
            bgfx::dbgTextClear();
            bgfx::dbgTextImage(
                /*x=*/      static_cast<uint16_t>(bx::max((int32_t)(width / 2 / k_DbgTextWidth) - (int32_t)(Logo::k_ImageWidth / 2), 0)),
                /*y=*/      static_cast<uint16_t>(bx::max((int32_t)(height / 2 / k_DbgTextHeight) - (int32_t)(Logo::k_ImageHeight / 2), 0)),
                /*width=*/  static_cast<uint16_t>(Logo::k_ImageWidth),
                /*height=*/ static_cast<uint16_t>(Logo::k_ImageHeight),
                /*data=*/   Logo::k_ImageData,
                /*pitch=*/  static_cast<uint16_t>(Logo::k_BufferBytesPerPixel * Logo::k_BufferWidth)
            );

            const bgfx::Stats* stats = bgfx::getStats();

            RecordSample(stats);

            char fpsOverlay[256];
            uint16_t fpsOverlayLength = static_cast<uint16_t>(bx::snprintf(fpsOverlay, BX_COUNTOF(fpsOverlay), "l:%.3fms, h:%.3fms a: %.3fms, %.1f FPS"
                , m_Samples.m_min
                , m_Samples.m_max
                , m_Samples.m_avg
                , 1000.0f / m_Samples.m_avg
            ));

            char resolutionOverlay[256];
            uint16_t resolutionOverlayLength = static_cast<uint16_t>(
                bx::snprintf(resolutionOverlay, BX_COUNTOF(resolutionOverlay), "%dW x %dW",
                stats->width, stats->height
            ));

            // try to keep the text in the same position, but make sure we show all of it.
            fpsOverlayLength = std::max<uint16_t>(fpsOverlayLength + 1, 43);
            resolutionOverlayLength = std::max<uint16_t>(resolutionOverlayLength + 1, 12);

            bgfx::dbgTextPrintf(stats->textWidth - fpsOverlayLength, 1, 0x0f, fpsOverlay);
            bgfx::dbgTextPrintf(stats->textWidth - resolutionOverlayLength, 2, 0x0f, resolutionOverlay);

            switch (m_DebugMode)
            {
            default:
            case DebugMode::None:
                bgfx::setDebug(BGFX_DEBUG_NONE);
                break;
            case DebugMode::Profiler:
                bgfx::setDebug(BGFX_DEBUG_PROFILER);
                break;
            case DebugMode::Stats:
                bgfx::setDebug(BGFX_DEBUG_STATS);
                break;
            case DebugMode::Text:
                bgfx::setDebug(BGFX_DEBUG_TEXT);
                break;
            case DebugMode::Wireframe:
                bgfx::setDebug(BGFX_DEBUG_WIREFRAME);
                break;
            }

            // wait until the main thread is done submitting draw calls
            m_SubmitFrameSemaphore.wait();
            // render everything that was submitted
            m_OnRender.Invoke();
            renderFrameNumber = bgfx::frame();

            // This moment here can be thought of as between frames.
            // This is where it is safe to update data shared between the main and render thread
            // because we know the main thread is waiting on us to post the render frame semaphore.
            const size_t prevStatsIndex = m_StatsCopyIndex;
            m_StatsCopyIndex = (m_StatsCopyIndex + 1) % 2;
            bx::memCopy(&m_StatsCopy[prevStatsIndex], &m_StatsCopy[m_StatsCopyIndex], sizeof(m_StatsCopy[0]));
            bx::memCopy(&m_StatsCopy[m_StatsCopyIndex], stats, sizeof(m_StatsCopy[0]));

            // clear the encoder stats for now since we don't have space reserved for them.
            m_StatsCopy[m_StatsCopyIndex].numEncoders = 0;
            m_StatsCopy[m_StatsCopyIndex].encoderStats = nullptr;

            // post so the main thread can begin submitting render calls for a new frame
            m_RenderFrameSemaphore.post();
        }

        bgfx::shutdown();
        return 0;
    }

    void RenderEngineImpl::Resize(uint32_t width, uint32_t height)
    {
        m_RenderThreadEvents.push(new ResizeEvent(width, height));
    }

    void RenderEngineImpl::SetDebugMode(DebugMode mode)
    {
        m_RenderThreadEvents.push(new DebugModeEvent(mode));
    }

    RenderEngine::RenderCallback& RenderEngineImpl::OnRender()
    {
        return m_OnRender;
    }

    const bgfx::Stats* RenderEngineImpl::GetStats() const
    {
        return &m_StatsCopy[m_StatsCopyIndex];
    }

    ////////////////////////////////////////////////////////////////////////// Private
    void RenderEngineImpl::ResetSamples()
    {
        m_Samples.m_offset = 0;
        bx::memSet(m_Samples.m_values, 0, sizeof(m_Samples.m_values));
        m_Samples.m_min = 0.0f;
        m_Samples.m_max = 0.0f;
        m_Samples.m_avg = 0.0f;
    }

    void RenderEngineImpl::RecordSample(const bgfx::Stats* stats)
    {
        const double toMsCpu = 1000.0 / stats->cpuTimerFreq;
        //const double toMsGpu = 1000.0 / stats->gpuTimerFreq;
        const double frameMs = double(stats->cpuTimeFrame) * toMsCpu;

        m_Samples.m_values[m_Samples.m_offset] = static_cast<float>(frameMs);
        m_Samples.m_offset = (m_Samples.m_offset + 1) % m_Samples.k_NumSamples;

        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::min();
        float avg = 0.0f;

        for (uint32_t ii = 0; ii < m_Samples.k_NumSamples; ++ii)
        {
            const float val = m_Samples.m_values[ii];
            min = val < min ? val : min;
            max = val > max ? val : max;
            avg += val;
        }

        m_Samples.m_min = min;
        m_Samples.m_max = max;
        m_Samples.m_avg = avg / m_Samples.k_NumSamples;
    }
}