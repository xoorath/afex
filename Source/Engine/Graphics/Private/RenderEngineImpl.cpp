#include "RenderEngineImpl.h"

// Graphics
#include "bgfxCallbacks.h"
#include "Event.h"
#include "ImguiRenderer.h"
#include "Logo.h"

// Engine
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>
#include <Graphics/DebugMode.h>
#include <Graphics/RenderEngineArgs.h>

// External
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>

// System
#include <atomic>
#include <limits>

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    ////////////////////////////////////////////////////////////////////////// internal
    RenderEngineImpl::RenderEngineImpl(const RenderEngineArgs& args)
        : m_ImguiRenderer()
        , m_QueueAllocator()
        , m_RenderThreadEvents(&m_QueueAllocator)
        , m_DebugMode(DebugMode::Default)
        , m_RenderThread()
        , m_Args(args)
        , m_Valid(false)
        , m_InitSemaphore()
        , m_MainToRenderSem()
        , m_RenderToMainSem()
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
        if (m_RenderThread.isRunning())
        {
            m_RenderThreadEvents.push(new ExitEvent());
            m_MainToRenderSem.post();
            m_RenderToMainSem.wait();
            while (bgfx::RenderFrame::NoContext != bgfx::renderFrame(k_ShutdownTimeoutMilliseconds))
            {
            }
            m_RenderThread.shutdown();
            AFEX_ASSERT_MSG(m_RenderThread.getExitCode() == 0, "Render thread shutdown was not successful");
            AFEX_LOG_TRACE("Render thread stopped. Exit code: {}", m_RenderThread.getExitCode());
        }
    }

    void RenderEngineImpl::BeginFrame() const
    {
        m_ImguiRenderer.BeginFrame();
    }

    void RenderEngineImpl::EndFrame() const
    {
        m_ImguiRenderer.EndFrame();
        bgfx::renderFrame(k_RenderTimeoutMilliseconds);

        // Create a sync point so we render and update once per frame
        m_MainToRenderSem.post();
        m_RenderToMainSem.wait();
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

        m_ImguiRenderer.Init(m_Args.GetImguiContext(), m_Args.GetWidth(), m_Args.GetHeight());
        m_InitSemaphore.post();

        constexpr bgfx::ViewId k_ClearView = 0;
        bgfx::setViewClear(k_ClearView, BGFX_CLEAR_COLOR, 0x1a1111ff);
        bgfx::setViewRect(k_ClearView, 0, 0, bgfx::BackbufferRatio::Equal);

        uint32_t width = m_Args.GetWidth();
        uint32_t height = m_Args.GetHeight();

        // font size in pixels
        const uint32_t k_DbgTextWidth = 8;
        const uint32_t k_DbgTextHeight = 16;

        bool exit = false;
        while (!exit)
        {
            // This cast to EventType is relying on the fact that all true message types
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
                    bgfx::setViewRect(k_ClearView, 0, 0, bgfx::BackbufferRatio::Equal);
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

            m_ImguiRenderer.Render();

            bgfx::touch(k_ClearView);
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
            bgfx::dbgTextPrintf(0, 0, 0x0f, "[F1] None  [F2] Text  [F3] Stats  [F4] Wireframe  [F5] Profiler");
            bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");
            bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
            bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");
            bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters.", stats->width, stats->height, stats->textWidth, stats->textHeight);

            switch (m_DebugMode)
            {
            default:
            case DebugMode::None:
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

            bgfx::frame();

            m_RenderToMainSem.post();
            if (!exit)
            {
                m_MainToRenderSem.wait();
            }
        }

        m_ImguiRenderer.Shutdown();

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
}