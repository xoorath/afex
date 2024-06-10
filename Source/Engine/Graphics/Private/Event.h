#pragma once

// Engine
#include <Graphics/DebugMode.h>

// System
#include <cstdint>

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    //////////////////////////////////////////////////////////////////////////
    // The first member of each event type should be the message type. 
    // This will allow us to queue any event type and first cast it to 
    // MessageDataType. Once the type has been determined we can cast 
    // to the concrete type to get the data for that event.
    enum class EventType : uint8_t
    {
        Exit,
        Resize,
        // Debug events:
        DebugMode,
        DebugLogo,

    };

    //////////////////////////////////////////////////////////////////////////
    struct ExitEvent
    {
    private:
        EventType m_Type = EventType::Exit;
    };

    //////////////////////////////////////////////////////////////////////////
    struct ResizeEvent
    {
    public:
        ResizeEvent(uint32_t width, uint32_t height) 
            : m_Type(EventType::Resize)
            , m_Width(width)
            , m_Height(height) { }
        ResizeEvent()                                       = default;
        ResizeEvent(const ResizeEvent&)                     = default;
        ResizeEvent(ResizeEvent&&) noexcept                 = default;
        ResizeEvent& operator=(const ResizeEvent&)          = default;
        ResizeEvent& operator=(ResizeEvent&&) noexcept      = default;
        ~ResizeEvent()                                      = default;

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
    private:
        EventType m_Type = EventType::Resize;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

    //////////////////////////////////////////////////////////////////////////
    struct DebugModeEvent
    {
    public:
        explicit DebugModeEvent(DebugMode mode) 
            : m_Type(EventType::DebugMode)
            , m_DebugMode(mode) { }
        DebugModeEvent()                                        = default;
        DebugModeEvent(const DebugModeEvent&)                   = default;
        DebugModeEvent(DebugModeEvent&&) noexcept               = default;
        DebugModeEvent& operator=(const DebugModeEvent&)        = default;
        DebugModeEvent& operator=(DebugModeEvent&&) noexcept    = default;
        ~DebugModeEvent()                                       = default;

        DebugMode GetDebugMode() const { return m_DebugMode; }
    private:
        EventType m_Type = EventType::DebugMode;
        DebugMode m_DebugMode = DebugMode::Default;
    };

    //////////////////////////////////////////////////////////////////////////
    struct DebugLogoEvent
    {
    public:
        explicit DebugLogoEvent(bool displayLogo)
            : m_Type(EventType::DebugLogo)
            , m_DisplayLogo(displayLogo) { }
        DebugLogoEvent() = default;
        DebugLogoEvent(const DebugLogoEvent&) = default;
        DebugLogoEvent(DebugLogoEvent&&) noexcept = default;
        DebugLogoEvent& operator=(const DebugLogoEvent&) = default;
        DebugLogoEvent& operator=(DebugLogoEvent&&) noexcept = default;
        ~DebugLogoEvent() = default;

        bool ShouldDisplayLogo() const { return m_DisplayLogo; }
    private:
        EventType m_Type = EventType::DebugLogo;
        bool m_DisplayLogo = false;
    };
}
