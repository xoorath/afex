#include <Graphics/RenderEngine.h>

// Graphics
#include "bgfxCallbacks.h"
#include "Event.h"
#include "Logo.h"

// Engine
#include <Core/Assert.h>
#include <Core/CommonMacros.h>
#include <Core/Logging.h>
#include <Graphics/DebugMode.h>

// External
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/handlealloc.h>
#include <bx/spscqueue.h>
#include <bx/thread.h>
#include <bx/mutex.h>

// System
#include <atomic>

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    ////////////////////////////////////////////////////////////////////////// internal
    class RenderEngineImpl
    {
    public:
        explicit RenderEngineImpl(const RenderEngineArgs& args);
        ~RenderEngineImpl();

        static int32_t StaticRenderThreadFunc(bx::Thread* thread, void* userData);
        int32_t RenderThreadFunc();

        bool IsValid() const { return m_Valid; }

        void Resize(uint32_t width, uint32_t height);
        void SetDebugMode(DebugMode mode);

    private:
        // Event queue
        bx::DefaultAllocator m_QueueAllocator;
        bx::SpScUnboundedQueue m_RenderThreadEvents;

        // Event data cache
        DebugMode m_DebugMode;

        // Lifetime management
        bx::Thread m_RenderThread;
        const RenderEngineArgs m_Args;  // used during setup
        bool m_Valid;                   // used during setup
    };

    ////////////////////////////////////////////////////////////////////////// RenderEngineArgs public
    /*GRAPHICS_EXPORT*/ RenderEngineArgs::RenderEngineArgs(void* nativeWindowHandle, uint32_t width, uint32_t height)
        : m_NativeWindowHandle(nativeWindowHandle)
        , m_Width(width)
        , m_Height(height)
    {
    }

    /*GRAPHICS_EXPORT*/ RenderEngineArgs::RenderEngineArgs()                                        = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs::RenderEngineArgs(const RenderEngineArgs&)                 = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs& RenderEngineArgs::operator=(const RenderEngineArgs&)      = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs::RenderEngineArgs(RenderEngineArgs&&) noexcept             = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs& RenderEngineArgs::operator=(RenderEngineArgs&&) noexcept  = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs::~RenderEngineArgs()                                       = default;

    /*internal*/ void* RenderEngineArgs::GetNativeWindowHandle() const
    {
        return m_NativeWindowHandle;
    }

    /*internal*/ uint32_t RenderEngineArgs::GetWidth() const
    {
        return m_Width;
    }

    /*internal*/ uint32_t RenderEngineArgs::GetHeight() const
    {
        return m_Height;
    }

    ////////////////////////////////////////////////////////////////////////// RenderEngine public
    /*GRAPHICS_EXPORT*/ RenderEngine::RenderEngine(const RenderEngineArgs& args)
    {
        RenderEngineImpl* impl = new RenderEngineImpl(args);
        if(!impl->IsValid())
        {
            delete impl;
            impl = nullptr;
        }
        m_PIMPL = reinterpret_cast<void*>(impl);
    }

    /*GRAPHICS_EXPORT*/ RenderEngine::~RenderEngine()
    {
        Shutdown();
    }

    /*GRAPHICS_EXPORT*/ RenderEngine::RenderEngine(RenderEngine&& other) noexcept
        : m_PIMPL(other.m_PIMPL)
    {
        other.m_PIMPL = nullptr; 
    }

    /*GRAPHICS_EXPORT*/ RenderEngine& RenderEngine::operator=(RenderEngine&& other) noexcept
    { 
        m_PIMPL = other.m_PIMPL;
        other.m_PIMPL = nullptr;
        return *this;
    }

    /*GRAPHICS_EXPORT*/ bool RenderEngine::IsValid() const
    {
        // Note: If initialization of the RenderEngineImpl fails, it will be null after construction
        return m_PIMPL != nullptr;
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::Shutdown()
    {
        delete reinterpret_cast<RenderEngineImpl*>(m_PIMPL);
        m_PIMPL = nullptr;
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::RenderFrame() const
    {
        bgfx::renderFrame(static_cast<int32_t>(k_RenderTimeoutMilliseconds));
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::Resize(uint32_t width, uint32_t height)
    {
        reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->Resize(width, height);
    }

    /*GRAPHICS_EXPORT*/ void RenderEngine::SetDebugMode(DebugMode mode)
    {
    reinterpret_cast<RenderEngineImpl*>(m_PIMPL)->SetDebugMode(mode);
    }

    ////////////////////////////////////////////////////////////////////////// internal
    RenderEngineImpl::RenderEngineImpl(const RenderEngineArgs& args)
        : m_QueueAllocator()
        , m_RenderThreadEvents(&m_QueueAllocator)
        , m_DebugMode(DebugMode::Default)
        , m_RenderThread()
        , m_Args(args)
        , m_Valid(false)
    {
        // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
        // Most graphics APIs must be used on the same thread that created the window.
        bgfx::renderFrame();

        if(!m_RenderThread.init(
            /*fn=*/         &RenderEngineImpl::StaticRenderThreadFunc,
            /*userData=*/   reinterpret_cast<void*>(this),
            /*stackSize=*/  RenderEngine::k_RenderThreadStackSize,
            /*name=*/       "AFEX Render Thread"
        ))
        {
            AFEX_LOG_ERROR("Failed to create the render thread");
            return;
        }

        // Only set true once the render thread has started
        m_Valid = true;
    }

    RenderEngineImpl::~RenderEngineImpl()
    {
        if (m_RenderThread.isRunning())
        {
            m_RenderThreadEvents.push(new ExitEvent());
            while (bgfx::RenderFrame::NoContext != bgfx::renderFrame(RenderEngine::k_ShutdownTimeoutMilliseconds)) 
            {
            }
            m_RenderThread.shutdown();
            AFEX_ASSERT_MSG(m_RenderThread.getExitCode() == 0, "Render thread shutdown was not successful");
            AFEX_LOG_TRACE("Render thread stopped. Exit code: {}", m_RenderThread.getExitCode());
        }
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

        if(!bgfx::init(init))
        {
            AFEX_LOG_ERROR("(RenderThread) bgfx::init failed");
            return -1;
        }

        constexpr bgfx::ViewId k_ClearView = 0;
        bgfx::setViewClear(k_ClearView, BGFX_CLEAR_COLOR, 0x1a1111ff);
        bgfx::setViewRect(k_ClearView, 0, 0, bgfx::BackbufferRatio::Equal);

        uint32_t width = m_Args.GetWidth();
        uint32_t height = m_Args.GetHeight();

        // font size in pixels
        const uint32_t k_DbgTextWidth = 8;
        const uint32_t k_DbgTextHeight = 16;

        bool exit = false;
        while(!exit)
        {
            // This cast to EventType is relying on the fact that all true message types
            // have an EventType member as the first member. See Event.h for more info.
            while(EventType* ev = reinterpret_cast<EventType*>(m_RenderThreadEvents.pop()))
            {
                switch(*ev)
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

            bgfx::touch(k_ClearView);
            bgfx::dbgTextClear();
            bgfx::dbgTextImage(
                /*x=*/      static_cast<uint16_t>(bx::max((int32_t)(width/2/k_DbgTextWidth) - (int32_t)(Logo::k_ImageWidth/2), 0)),
                /*y=*/      static_cast<uint16_t>(bx::max((int32_t)(height/2/k_DbgTextHeight) - (int32_t)(Logo::k_ImageHeight/2), 0)),
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

            switch(m_DebugMode)
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
}